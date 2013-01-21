<?php
/**
 * @file
 * The mediamose storage s3 wrapper handles files that are either in transition
 * or on the S3 storage.
 */

class mediamosa_storage_s3_streamwrapper extends mediamosa_io_streamwrapper_external {
  // ------------------------------------------------------------------- Consts.
  const PROTOCOL = 's3';

  // --------------------------------------------------------------------- Vars.
  /**
   * Directory listing.
   *
   * @var array
   */
  public $file_list = null;

  /**
   * Stored position in file_list.
   *
   * @var integer.
   */
  public $position = 0;

  /**
   * End of file.
   *
   * @var boolean
   */
	public $eof = FALSE;

  /**
   * Open file handle.
   *
   * @var string
   */
  public $open_file = NULL;

  /**
   * Current position of the stream
   *
   * @var integer
   */
	public $seek_position = 0;

  /**
   * Temporary buffer.
   *
   * @todo: need rewrite with tmpfile().
   *
   * @var string
   */
	public $buffer = NULL;

  /**
   * File size.
   *
   * @var integer.
   */
	public $object_size = 0;

  // --------------------------------------------------------- Abstract methods.
  /**
   * Build the storage object that belongs to this streamwrapper.
   *
   * @param array $storage_profile
   *   Storage profile.
   *
   * @return mediamosa_storage_s3
   *   The mediamosa storage S3 object.
   */
  public function create_storage_object(array $storage_profile) {
    return new mediamosa_storage_s3($storage_profile);
  }

  /**
   * Get MD5 from s3 file.
   *
   * @return string
   *   The MD5 of the file or FALSE otherwise.
   */
  public function md5_file() {
    $md5s = &drupal_static(self::STATIC_MD5S, array());
    assert(!empty($this->uri));

    if (!empty($md5s[$this->uri]) && $md5s[$this->uri] != self::MD5_DEFAULT) {
      return $md5s[$this->uri];
    }

    // We should trust technical metadata.
    $mediafile_id = $this->get_mediafile_id();

    // Get technical metadata MD5.
    $md5s[$this->uri] = mediamosa_asset_mediafile::get_md5($mediafile_id);

    // Technical MD5 is missing, fallback to md5 from S3.
    if (empty($md5s[$this->uri])) {
      // Need s3 object.
      $s3 = $this->get_s3();

      // Get bucket.
      $bucket = $this->get_bucket();

      // Get mediafile path.
      $medaifile_path = $this->get_mediafile_path();

      // Get MD5.
      $md5s[$this->uri] = $s3->_getMD5($bucket, $medaifile_path);

      // Still empty? Set to default (as unknown).
      if (empty($md5s[$this->uri])) {
        $md5s[$this->uri] = self::MD5_DEFAULT;
      }
    }

    return $md5s[$this->uri];
  }

  /**
   * Get the file on an local location and return the location.
   *
   * Our file is on S3 storage. But might be already in the transition storage.
   * Either use the transition storage version or download the file and store
   * it in transition.
   *
   * @return string
   *   Returns the uri to the local file.
   */
  public function get_uri_local_file() {
    // Need transition uri.
    $transition_uri = mediamosa_storage_transition::get_transition_uri($this->uri);

    // Check if the file already exists.
    if (!mediamosa_io::file_exists($transition_uri)) {
      // Ok, file does not exists, we need to download.
      //
      // Need s3 object.
      $s3 = $this->get_s3();

      // Get bucket.
      $bucket = $this->get_bucket();

      // Get path.
      $path = $this->get_mediafile_path();

      // Need real path.
      $transition_path = mediamosa_io::realpath($transition_uri);
      if (empty($transition_path)) {
        throw new mediamosa_storage_s3_exception('Failed to get transition space to download file.');
      }

      // Now download.
      if (!$s3->_download($bucket, $path, $transition_path)->isOK()) {
        // Failed, throw.
        throw new mediamosa_storage_s3_exception('Failed to download file.');
      }

      if (!mediamosa_io::file_exists($transition_uri)) {
        // Failed, throw.
        throw new mediamosa_storage_s3_exception('Failed to download file, file was not found.');
      }
    }

    // Check md5 download.
    $md5 = $this->md5_file();

    // If md5 is not present, then skip the check. Accept as is.
    if (!empty($md5)) {
      // Get md5.
      $md5_source = mediamosa_io::md5_file($transition_uri);

      // Check md5.
      if ($md5_source != $md5) {
        throw new mediamosa_storage_s3_exception(t("Download failed: file does not match stored md5: '@md5'. Downloaded file got '@md5_source'.", array('@md5' => $md5, '@md5_source' => $md5_source)));
      }
    }

    return $transition_uri;
  }

  /**
   * Upload local file to destination (us).
   *
   * Function must make sure that on return $this->uri is updated with new
   * location and $this->md5 is set.
   *
   * @param mediamosa_io_streamwrapper $mediamosa_io_streamwrapper_source
   *   The local stream wrapper.
   *
   * @throws mediamosa_exception_error
   */
  public function mediafile_copy($mediamosa_io_streamwrapper_source) {
    // First get source file as local.
    $source_uri = $mediamosa_io_streamwrapper_source->get_uri_local_file();

    // Make sure source exists.
    if (!mediamosa_io::file_exists($source_uri)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("source file '@uri' (@path) was not found.", array('@uri' => $source_uri, '@path' => mediamosa_io::realpath($source_uri)))));
    }
    elseif (!mediamosa_io::is_file($source_uri)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("source file '@uri' (@path) is not a file.", array('@uri' => $source_uri, '@path' => mediamosa_io::realpath($source_uri)))));
    }

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $path = mediamosa_io::dirname($this->get_mediafile_path());

    // Need mediafile_id as filename.
    $mediafile_id = $this->get_mediafile_id();

    // Need MD5 of local file.
    $source_md5 = mediamosa_io::md5_file($source_uri);

    // Mediafile must exist in db.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Need content_type.
    $content_type = mediamosa_asset_mediafile::get_mime_type($mediafile_id);

    // Check if mediafile is public.
    $acl = mediamosa_storage_s3::mediafile_is_public($mediafile) ? MediaMosaAmazonS3::ACL_PUBLIC : MediaMosaAmazonS3::ACL_PRIVATE;

    // Upload the file.
    $s3->_upload($bucket, $source_uri, $path, $source_md5, $mediafile_id, $acl, $content_type);

    // Filename might not match because of md5 name in transition_uri, and
    // upload is with destination path.
    $filename_source = mediamosa_io::basename($source_uri);
    $filename_dest = $this->get_mediafile_id();
    if ($filename_source != $filename_dest) {
      $response = $s3->copy_object(
        array('bucket' => $bucket, 'filename' => $path . '/' . $filename_source),
        array('bucket' => $bucket, 'filename' => $path . '/' . $filename_dest)
      );

      if ($response->isOK()) {
        return $s3->delete_object($bucket, $path . '/' . $filename_source)->isOK();
      }
    }
// @todo: remove from release.
    mediamosa_debug::log_export('debug in ' . __FUNCTION__ . '() on line ' . __LINE__ . ' in ' . __FILE__);
    mediamosa_debug::log_export($this->uri);
// @todo: remove from release.

    if (!$this->file_exists()) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("destination file '@uri' (@path) was not found; move failed.", array('@uri' => $source_uri, '@path' => mediamosa_io::realpath($source_uri)))));
    }

    // Set the MD5.
    $this->setMD5($source_md5);
  }

  /**
   * Process media view request.
   *
   * @param integer $app_id
   *   Application ID.
   * @param string $user_id
   *   The owner for possible ticket.
   * @param array $mediafile
   *   The mediafile to process.
   * @param string $response_type
   *   See mediamosa_media::RESPONSE_*
   * @param boolean $is_app_admin
   *   Is app admin.
   *
   * @return array
   *   An associative array; (see mediamosa_io_streamwrapper::MEDIA_VIEW_*)
   *   - 'ticket_id'
   *     The ticket ID used.
   *   - 'link'
   *     The symbolic link complete path.
   *   - 'filename'
   *     The filename of the media.
   *   - 'server_uri_build'
   *     The finished server uri.
   *   - 'is_public'
   *     Public urls have no ACL rules.
   */
   public function media_view($app_id, $user_id, array $mediafile, $response_type, $is_app_admin) {

    // Get the stuff we need.
    $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];

    // Get the permanent status, public mediafiles can be public.
    $is_public = mediamosa_media::is_public($mediafile, $is_app_admin);

    // Get the filename.
    $filename = mediamosa_media::get_media_filename($mediafile);

    // Convert to ticket type.
    $ticket_type = mediamosa_media::responsetype2tickettype($response_type);

    // Is a public link?
    if ($is_public) {
      $ticket_id = $mediafile_id;
    }
    else {
      // Create ticket.
      $ticket_id = mediamosa_media_ticket::create($app_id, $ticket_type, $user_id, $mediafile_id);
    }

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get mediafile path.
    $filename = $this->get_mediafile_path();

    // Need expire time (in minutes).
    switch ($ticket_type) {
      case mediamosa_media_ticket::TICKET_TYPE_DOWNLOAD:
        $expire = mediamosa_media_ticket::get_ticket_expiration_download();
        break;

      case mediamosa_media_ticket::TICKET_TYPE_STILL:
        $expire = mediamosa_media_ticket::get_ticket_expiration_still();
        break;

      default:
      case mediamosa_media_ticket::TICKET_TYPE_VIEW:
        $expire = mediamosa_media_ticket::get_ticket_expiration_view();
        break;
    }

    // Convert to UTC since.
    $expire = gmmktime() + ($expire * 60);

    // Get play url.
    $url = $s3->get_object_url($bucket, $filename, $expire);

    // Return the information needed.
    return array(
      mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID => $ticket_id,
      mediamosa_io_streamwrapper::MEDIA_VIEW_FILENAME => $filename,
      mediamosa_io_streamwrapper::MEDIA_VIEW_LINK => FALSE,
      mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD => $url,
      mediamosa_io_streamwrapper::MEDIA_VIEW_IS_PUBLIC => $is_public,
    );
  }

  // ------------------------------------------------------ Abstract IO Methods.
	/**
	 * Open directory handle.
   *
   * This method is called in response to php:opendir().
	 *
	 * @param string $uri
   *   Specifies the URL that was passed to opendir().
	 * @param integer $options
   *   (ignored) Whether or not to enforce safe_mode (0x04).
   *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function dir_opendir($uri, $options) {
    // Store uri.
    $this->uri = $uri;

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $object_name = $this->get_mediafile_path();

    // Get directory listing.
    $this->file_list = $s3->get_object_list(
      $bucket,
      array(
        'pcre' => '/^' . $this->quotemeta($object_name) . '(.*)[^\/$]/',
      )
    );

		return !empty($this->file_list) ? TRUE : FALSE;
	}

	/**
	 * Close directory handle.
   *
   * This method is called in response to php:closedir().
	 *
	 * @return boolean
	 *   Since Amazon S3 doesn't have real directories, always return TRUE.
	 */
	public function dir_closedir() {
		$this->position = 0;
		$this->uri = NULL;
		$this->file_list = NULL;
		$this->open_file = NULL;
		$this->seek_position = 0;
		$this->eof = FALSE;
		$this->buffer = NULL;
		$this->object_size = 0;
		return TRUE;
	}

	/**
   * Read entry from directory handle.
   *
	 * This method is called in response to php:readdir().
	 *
	 * @return string
   *   Should return a string representing the next filename, or FALSE if there
   *   is no next file.
	 */
	public function dir_readdir() {
		if (isset($this->file_list[$this->position]))	{
			return $this->file_list[$this->position++];
    }

		return FALSE;
	}

	/**
   * Rewind directory handle.
   *
	 * This method is called in response to php:rewinddir().
	 *
	 * Should reset the output generated by php:streamWrapper::dir_readdir().
   * i.e.: The next call to php:streamWrapper::dir_readdir() should return the
   * first entry in the location returned by php:streamWrapper::dir_opendir().
	 *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function dir_rewinddir() {
		$this->position = 0;
		return TRUE;
	}

	/**
	 * This method is called in response to php:flock(), when
   * php:file_put_contents() (when flags contains LOCK_EX),
   * php:stream_set_blocking() and when closing the stream (LOCK_UN).
	 *
	 * Not implemented in S3, so it's not implemented here.
	 *
	 * @param mode $operation
   *   Is one of the following:
   *   - LOCK_SH
   *     To acquire a shared lock (reader).
   *   - LOCK_EX
   *     To acquire an exclusive lock (writer).
   *   - LOCK_UN
   *     To release a lock (shared or exclusive).
   *   - LOCK_NB
   *     If you don't want flock() to block while locking. (not supported on
   *     Windows).
   *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function stream_lock($operation) {
    return TRUE;
  }

	/**
	 * Opens file or URL.
   *
   * This method is called immediately after the wrapper is initialized (e.g.,
   * by php:fopen() and php:file_get_contents()).
	 *
	 * @param string $uri
   *   Specifies the URL that was passed to the original function.
	 * @param string $mode
   *   (ignored) The mode used to open the file, as detailed for fopen().
	 * @param integer $options
   *   (ignored) Remember to check if the mode is valid for the path requested.
	 * @param string &$opened_path
   *   If the path is opened successfully, and STREAM_USE_PATH is set in
   *   options, opened_path should be set to the full path of the file/resource
   *   that was actually opened.
   * .
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function stream_open($uri, $mode, $options, &$opened_path) {
		$opened_path = $uri;
		$this->open_file = $uri;
		$this->uri = $uri;
		$this->seek_position = 0;
		$this->object_size = 0;

		return TRUE;
	}

	/**
	 * Close an resource.
   *
   * This method is called in response to php:fclose().
	 *
	 * All resources that were locked, or allocated, by the wrapper should be
   * released.
	 */
	public function stream_close() {
		$this->position = 0;
		$this->path = NULL;
		$this->file_list = NULL;
		$this->open_file = NULL;
		$this->seek_position = 0;
		$this->eof = FALSE;
		$this->buffer = NULL;
		$this->object_size = 0;
	}

	/**
	 * Read from stream.
   *
   * This method is called in response to php:fread() and php:fgets().
	 *
	 * @param integer $count
   *   How many bytes of data from the current position should be returned.
   *
	 * @return string
   *   The contents of the Amazon S3 object.
	 */
	public function stream_read($count)	{
		if ($this->eof)	{
			return FALSE;
		}

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $object_name = $this->get_mediafile_path();

		if ($this->seek_position > 0 && $this->object_size)	{
			if ($count + $this->seek_position > $this->object_size) {
				$count = $this->object_size - $this->seek_position;
			}

			$start = $this->seek_position;
			$end = $this->seek_position + $count;

			$response = $s3->get_object($bucket, $object_name, array(
				'range' => $start . '-' . $end
			));
		}
		else {
			$response = $s3->get_object($bucket, $object_name);
			$this->object_size = isset($response->header['content-length']) ? $response->header['content-length'] : 0;
		}

		if (!$response->isOK())	{
			return FALSE;
		}

		$data = substr($response->body, 0, min($count, $this->object_size));
		$this->seek_position += strlen($data);

		if ($this->seek_position >= $this->object_size) {
      $this->eof = TRUE;
			$this->seek_position = 0;
			$this->object_size = 0;
		}

		return $data;
	}

	/**
	 * Seeks to specific location in a stream.
   *
   * This method is called in response to php:fseek(). The read/write
	 * position of the stream should be updated according to the $offset and
   * $whence parameters.
   *
	 * @param integer $offset
   *   The number of bytes to offset from the start of the file.
	 * @param integer $whence
   *   (optional, ignored) Always uses SEEK_SET.
   *
	 * @return boolean
   *   Return TRUE if the position was updated, FALSE otherwise.
	 */
	public function stream_seek($offset, $whence) {
		$this->seek_position = $offset;
		return TRUE;
	}

  /**
	 * Retrieve information about a file resource.
	 *
	 * @return array
   *   Returns the same data as a call to php:stat().
	 */
	public function stream_stat() {
		return $this->url_stat($this->uri, NULL);
	}

	/**
	 * Retrieve the current position of a stream.
   *
   * This method is called in response to php:ftell().
	 *
	 * @return integer
   *   Returns the current position of the stream.
	 */
	public function stream_tell()	{
		return $this->seek_position;
	}

	/**
	 * Write to stream.
   *
   * This method is called in response to php:fwrite().
	 *
	 * It is important to avoid reading files that are larger than the amount of memory allocated to PHP,
	 * otherwise "out of memory" errors will occur.
	 *
	 * @param string $data
   *   The data to write to the stream.
   *
	 * @return integer
   *   The number of bytes that were written to the stream.
	 */
	public function stream_write($data) {
		$this->seek_position = strlen($data);
		$this->buffer .= $data;

		return $this->seek_position;
	}

	/**
	 * Flushes the output.
   *
   * This method is called in response to php:fflush(). If you have cached data
   * in your stream but not yet stored it into the underlying storage, you
   * should do so now.
	 *
	 * Since this implementation doesn't buffer streams, simply return TRUE.
	 *
	 * @return boolean
   *   If not implemented, FALSE is assumed as the return value.
	 */
	public function stream_flush() {
		if ($this->buffer === NULL) {
			return FALSE;
		}

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $object_name = $this->get_mediafile_path();

		$response = $s3->create_object($bucket, $object_name, array(
			'body' => $this->buffer,
		));

		$this->seek_position = 0;
		$this->buffer = NULL;
		$this->eof = TRUE;

		return $response->isOK();
	}

	/**
	 * Tests for end-of-file on a file pointer.
   *
   * This method is called in response to php:feof().
	 *
	 * @return boolean
	 */
	public function stream_eof() {
		return $this->eof;
	}

	/**
	 * Delete a file.
   *
   * This method is called in response to php:unlink().
	 *
	 * @param string $uri
   *   The file URI which should be deleted.
   *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function unlink($uri)	{
    // Make sure md5file is recalculated now this file is removed.
    $md5s = &drupal_static(self::STATIC_MD5S, array());
    unset($md5s[$uri]);

    // Store uri.
    $this->uri = $uri;

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $object_name = $this->get_mediafile_path();

    // Do delete.
		return $s3->delete_object($bucket, $object_name)->isOK();
	}

	/**
	 * Renames a file or directory.
   *
   * This method is called in response to php:rename(). Both uris must be on
   * the same location.
	 *
	 * @param string $path_from
   *   The URL to the current file.
	 * @param string $path_to
   *   The URL which the $path_from should be renamed to.
   *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function rename($path_from, $path_to) {
    // Need s3 object.
    $s3 = $this->get_s3();

    $from_bucket_name = $this->get_bucket($this->get_parts($path_from, 'path'));
    $to_bucket_name = $this->get_bucket($this->get_parts($path_to, 'path'));
    $from_object_name = $this->get_parts($path_from, 'mediafile_path');
    $to_object_name = $this->get_parts($path_to, 'mediafile_path');

		$response = $s3->copy_object(
			array('bucket' => $from_bucket_name, 'filename' => $from_object_name),
			array('bucket' => $to_bucket_name, 'filename' => $to_object_name)
		);

    if ($response->isOK()) {
			return $s3->delete_object($from_bucket_name, $from_object_name)->isOK();
		}

		return FALSE;
	}

	/**
	 * Create a new bucket.
   *
   * This method is called in response to php:mkdir().
	 *
	 * @param string $path
   *   The bucket name to create.
	 * @param integer $mode
   *   (optional) Permissions.
   *   700-range permissions map to ACL_PUBLIC.
   *   600-range permissions map to ACL_AUTH_READ.
   *   All other permissions map to ACL_PRIVATE.
	 * @param integer $options
   *   (optional, ignored) A bitwise mask of values, such as
   *   STREAM_MKDIR_RECURSIVE.
   *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function mkdir($path, $mode, $options)	{
    // Store uri.
    $this->uri = $path;

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Figure out acl.
  	$acl = AmazonS3::ACL_PRIVATE;
		if (in_array($mode, range(700, 799))) {
			$acl = AmazonS3::ACL_PUBLIC;
		}
		elseif (in_array($mode, range(600, 699))) {
			$acl = AmazonS3::ACL_AUTH_READ;
		}

    // TODO: region select.
		return $s3->create_bucket($bucket, AmazonS3::REGION_IRELAND, $acl)->isOK();
	}

	/**
   * Removes a directory.
   *
	 * This method is called in response to php:rmdir().
	 *
	 * @param string $path
   *   The bucket name to create.
	 * @param boolean $context
   *   (optional, ignored) A bitwise mask of values, such as
   *   STREAM_MKDIR_RECURSIVE.
   *
	 * @return boolean
   *   Returns TRUE on success or FALSE on failure.
	 */
	public function rmdir($path, $context) {
    // Store uri.
    $this->uri = $path;

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

		return $s3->delete_bucket($bucket)->isOK();
	}

	/**
   *  Retrieve information about a file.
   *
	 * This method is called in response to all php:stat() related functions.
	 *
	 * @param string $path
   *   The file path or URL to stat. Note that in the case of a URL, it must be
   *   a :// delimited URL. Other URL forms are not supported.
	 * @param integer $flags
   *   (ignored) Holds additional flags set by the streams API. It can hold one
   *   or more of the following values OR'd together.
   *   - STREAM_URL_STAT_LINK
   *     For resources with the ability to link to other resource (such as an
   *     HTTP Location: forward, or a filesystem symlink). This flag specified
   *     that only information about the link itself should be returned, not the
   *     resource pointed to by the link. This flag is set in response to calls
   *     to lstat(), is_link(), or filetype().
   *   - STREAM_URL_STAT_QUIET
   *     If this flag is set, your wrapper should not raise any errors. If this
   *     flag is not set, you are responsible for reporting errors using the
   *     trigger_error() function during stating of the path.
   *
	 * @return array
   *   Should return as many elements as stat() does. Unknown or unavailable
   *   values should be set to a rational value (usually 0).
	 */
	public function url_stat($path, $flags) {
  	// Defaults
		$out = array();
		$out[0] = $out['dev'] = 0;
		$out[1] = $out['ino'] = 0;
		$out[2] = $out['mode'] = 0;
		$out[3] = $out['nlink'] = 0;
		$out[4] = $out['uid'] = 0;
		$out[5] = $out['gid'] = 0;
		$out[6] = $out['rdev'] = 0;
		$out[7] = $out['size'] = 0;
		$out[8] = $out['atime'] = 0;
		$out[9] = $out['mtime'] = 0;
		$out[10] = $out['ctime'] = 0;
		$out[11] = $out['blksize'] = 0;
		$out[12] = $out['blocks'] = 0;

		$file = NULL;
		$mode = 0;

    // Store uri.
		$this->uri = $path;

    // Need s3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $object_name = $this->get_mediafile_path();

    if ($object_name) {
      // Filter double //.
      $object_name = str_replace('//', '/', $object_name);

      $response = $s3->list_objects($bucket, array(
				'prefix' => $object_name,
			));

			if (!$response->isOK())	{
				return $out;
			}

      // Go through the found file(s) and see if it matches the uri we seek.
      foreach ($response->body->Contents as $file) {
        if (!is_object($file)) {
          $file = simplexml_load_string($response->body)->Contents[0];
        }

        // Must match, or else its not what we are looking for.
        if ($object_name == (string) $file->Key) {
          break;
        }

        // See if we are looking for the directory.
        if ($object_name . '/' == substr((string) $file->Key, 0, strlen($object_name . '/'))) {
          // Yes we are. Fix it to directory.
          $file->Key = $object_name;
          $file->Size = 0;
          break;
        }
      }

      // Again, test. Must match or else its not what we want.
      if (!$file || $object_name != (string) $file->Key) {
        return FALSE; // File / dir not found.
      }
		}
		else {
			if (!$s3->list_objects($bucket)->isOK()) {
				return $out;
			}
		}

    //  Type & Permission bitwise values (only those that pertain to S3).
    //  Simulate the concept of a "directory". Nothing has an executable bit
    //  because there's no executing on S3.
    //  Reference: http://docstore.mik.ua/orelly/webprog/pcook/ch19_13.htm
    //
    //  0100000 => type:   regular file
    //  0040000 => type:   directory
    //  0000400 => owner:  read permission
    //  0000200 => owner:  write permission
    //  0000040 => group:  read permission
    //  0000020 => group:  write permission
    //  0000004 => others: read permission
    //  0000002 => others: write permission

		// File or directory?
    //
		// Root of the bucket?
    if ($object_name === '') {
			$mode = octdec('004066');
		}
		elseif ($file) {
      // If object name is the same and we have file size.
      $is_file = $object_name == (string) $file->Key && (int) $file->Size > 0;

      // Regular File, Owner R/W : Directory, Owner R/W.
			$mode = $is_file ? octdec('0100666') : octdec('0040666');
		}
		else {
			$mode = octdec('0100666');
		}

		// Update stat output.
		$out[2] = $out['mode'] = $mode;
		$out[4] = $out['uid'] = (isset($file) ? (string) $file->Owner->ID : 0);
		$out[7] = $out['size'] = (isset($file) ? (string) $file->Size : 0);
		$out[8] = $out['atime'] = (isset($file) ? date('U', strtotime((string) $file->LastModified)) : 0);
		$out[9] = $out['mtime'] = (isset($file) ? date('U', strtotime((string) $file->LastModified)) : 0);
		$out[10] = $out['ctime'] = (isset($file) ? date('U', strtotime((string) $file->LastModified)) : 0);

    return $out;
	}

  // ------------------------------------------------------------------ Methods.
  /**
   * Get and create the s3 object.
   *
   * @return MediaMosaAmazonS3
   *   The amazon object.
   */
  public function get_s3() {
    // Create S3 profile object.
    $storage_object = $this->create_storage_object($this->get_storage_profile());

    // Need s3 object.
    return $storage_object->create_s3();
  }

  /**
   * Return the bucket from the uri.
   *
   * @param string $uri
   *   (optional) The uri
   *
   * @return string
   *   The bucket.
   */
  public function get_bucket($uri = NULL) {
    // Get the path.
    $path = parent::get_path($uri);

    list($bucket, ) = explode('/', $path, 2);

    // First part should be the bucket.
    return $bucket;
  }

	/**
	 * Make token PCRE-compatible.
	 *
	 * @param string $str
   *   The token.
   *
	 * @return string
   *   The PCRE-compatible version of the token.
	 */
	public function quotemeta($str) {
		return str_replace('\\\\', '\\', quotemeta(str_replace('/', '\/', $str)));
	}

  /**
   * Test if the file exists.
   *
   * @return boolean
   *   Returns TRUE when file exists.
   */
  public function file_exists() {
    try {
      // Need S3 object.
      $s3 = $this->get_s3();

      // Get bucket.
      $bucket = $this->get_bucket();

      // Get path.
      $filename = $this->get_mediafile_path();

      // Get file status.
      if ($s3->if_object_exists($bucket, $filename)) {
        return TRUE;
      }

      // Stat the file.
      $stat = $this->url_stat($this->uri, STREAM_URL_STAT_QUIET);

      // Check the file flag.
      return (is_array($stat) && (($stat['mode'] & mediamosa_io::STAT_IFDIR) == mediamosa_io::STAT_IFDIR));
    }
    catch (Exception $e) {
      // If we fail, better not act on the exception. Log it.
      mediamosa_watchdog::log_mediafile($this->get_mediafile_id(), "Caught exception: unable test file exists for mediafile with ID: '@mediafile_id'; @message", array('@mediafile_id' => $this->get_mediafile_id(), '@message' => $e->getMessage()), WATCHDOG_ALERT);
    }

    return FALSE;
  }

  /**
   * Set ACL on S3.
   *
   * @param boolean $is_public
   *   Is either public or private.
   *
   * @return CFResponse
   *   The CFResponse or FALSE.
   */
  public function set_acl($is_public) {
    try {
      // Need S3 object.
      $s3 = $this->get_s3();

      // Get bucket.
      $bucket = $this->get_bucket();

      // Get path.
      $filename = $this->get_mediafile_path();

      // Set ACL on file.
      return $s3->set_object_acl($bucket, $filename, $is_public ? MediaMosaAmazonS3::ACL_PUBLIC : MediaMosaAmazonS3::ACL_PRIVATE);
    }
    catch (Exception $e) {
      // If we fail, better not act on the exception. Log it.
      mediamosa_watchdog::log_mediafile($this->get_mediafile_id(), "Caught exception: unable to set S3 ACL on mediafile with ID: '@mediafile_id'; @message", array('@mediafile_id' => $this->get_mediafile_id(), '@message' => $e->getMessage()), WATCHDOG_ALERT);
    }

    return FALSE;
  }

  /**
   * Get ACL status on S3.
   *
   * @return string
   *   Is either MediaMosaAmazonS3::ACL_PUBLIC or
   *   MediaMosaAmazonS3::ACL_PRIVATE.
   */
  public function get_acl() {
    // Need S3 object.
    $s3 = $this->get_s3();

    // Get bucket.
    $bucket = $this->get_bucket();

    // Get path.
    $filename = $this->get_mediafile_path();

    // Set ACL on file.
    $response = $s3->get_object_acl($bucket, $filename);

    $is_public = FALSE;
    foreach ($response->body->xpath('AccessControlList/Grant/Grantee') as $grantee) {
      if (isset($grantee->URI) && $grantee->URI == MediaMosaAmazonS3::USERS_ALL) {
        $is_public = TRUE;
        break;
      }
    }

    return $is_public ? MediaMosaAmazonS3::ACL_PUBLIC : MediaMosaAmazonS3::ACL_PRIVATE;
  }
}
