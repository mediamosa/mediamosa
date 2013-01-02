<?php
/**
 * @file
 * The streamwrapper external class is meant for storage that is not local, like
 * Amazon S3.
 *
 * Notes:
 * The uri of typical external mediamosa stream;
 *
 * Uri:
 * {scheme}://{storage_profile_id}[/path][/-/mediafile_path].
 *
 * Difference with local wrapper is that storage_profile_id in uri is required.
 */

abstract class mediamosa_io_streamwrapper_external extends mediamosa_io_streamwrapper {
  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Implements getDirectoryPath().
   */
  final public function getDirectoryPath() {
    throw new mediamosa_exception_program_error(__CLASS__ . ' or extended classes can not use getDirectoryPath().');
  }

  /**
   * External stream wrappers can not provide a local (realpath) file location,
   * unless the file exists in transition.
   *
   * Its required that the $this->uri is complete with mediafile ID.
   */
  public function realpath() {
    // External stream wrappers may use the version in transition, if we can
    // find it (and has correct md5).
    $transition_uri = mediamosa_storage_transition::find_transition_file($this);

    if (empty($transition_uri)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_EXTERNAL_FILE_NOT_FOUND_LOCAL, array('@uri' => $this->uri));
    }

    // Get the local path.
    return mediamosa_io::realpath($transition_uri);
  }

  /**
   * Overrides getExternalUrl().
   *
   * Return the HTML URI of a public file.
   */
  public function getExternalUrl() {
    // Unsupported for now.
    return '';

    //$path = str_replace('\\', '/', $this->getTarget());
    //return self::getDirectoryPath() . '/' . drupal_encode_path($path);
  }

  // --------------------------------------------------------- Abstract methods.
  /**
   * Validate the uri format.
   *
   * Expects;
   * {scheme}://{storage_profile_id}[/{path}][/-/{mediafile_path}]
   *
   * @param string $uri
   *   The uri to validate.
   *
   * @return array
   *   An associtieve array (optional items are either set or empty);
   *   - 'scheme'
   *     The wrapper scheme.
   *   - 'storage_profile_id'
   *     The storage profile ID.
   *   - 'path'
   *     (optional) The path.
   *   - 'mediafile_path'
   *     (optional) The mediafile path.
   *
   * @throws mediamosa_exception_error
   */
  public function validate_uri($uri) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    $matches = array();
    $reg = '#^(?P<scheme>.+)://(?P<strorage_profile_id>[0-9]+)/?(?:(?P<path>.*)/-/(?P<mediafile_path>.*)|(?P<altpath>.+))?$#';
    if (!preg_match($reg, $uri, $matches)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_IO_ERROR, array('@error' => "invalid storage uri: '" . $uri . "'"));
    }

    // Rename altpath.
    if (isset($matches['altpath'])) {
      $matches['path'] = $matches['altpath'];
      unset($matches['altpath']);
    }

    $mediafile_path = empty($matches['mediafile_path']) ? '' : trim($matches['mediafile_path'], '\/');

    // Validate the mediafile_path part.
    if (!empty($mediafile_path) && !$this->validate_uri_mediafile_path($mediafile_path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_IO_ERROR, array('@error' => "invalid storage uri: '" . $uri . "'"));
    }

    // Return parts.
    return array(
      'scheme' => $matches['scheme'],
      'storage_profile_id' => $matches['strorage_profile_id'],
      'path' => empty($matches['path']) ? '' : trim($matches['path'], '\/'),
      'mediafile_path' => $mediafile_path,
    );
  }

  /**
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id[0]/mediafile_id
   *
   * Remember that mediafile_id is optional.
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return boolean
   *   Returns TRUE for match, FALSE otherwise.
   */
  public function validate_uri_mediafile_path($mediafile_path) {
    $reg = '#^.*[a-zA-Z0-9](/[a-zA-Z0-9]+)?$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
  }

  /**
   * Test if the current path/file is local or external (like Amazon S3).
   *
   * @return boolean
   *   Returns TRUE when local or FALSE otherwise.
   */
  public function is_local() {
    return FALSE;
  }

  // --------------------------------------------------- Must implement methods.
  /**
   * These methods are base functions of the streamwrapper. But they can not be
   * declared as abstract, as these are already in our Drupal parent class.
   * However, I must force these to be created by extended classes.
   */

  /**
   * Support for mkdir().
   *
   * @param string $uri
   *   A string containing the URI to the directory to create.
   * @param integer $mode
   *   Permission flags - see mkdir().
   * @param integer $options
   *   A bit mask of STREAM_REPORT_ERRORS (unsupported) and
   *   STREAM_MKDIR_RECURSIVE.
   *
   * @return boolean
   *   TRUE if directory was successfully created.
   *
   * @see http://php.net/manual/en/streamwrapper.mkdir.php
   */
  public function mkdir($uri, $mode, $options) {
    throw new mediamosa_exception_program_error('Must implement ' . __FUNCTION__);
  }

  /**
   * Support for unlink().
   *
   * @param $uri
   *   A string containing the URI to the resource to delete.
   *
   * @return
   *   TRUE if resource was successfully deleted.
   *
   * @see http://php.net/manual/en/streamwrapper.unlink.php
   */
  public function unlink($uri) {
    throw new mediamosa_exception_program_error('Must implement ' . __FUNCTION__);
  }
}
