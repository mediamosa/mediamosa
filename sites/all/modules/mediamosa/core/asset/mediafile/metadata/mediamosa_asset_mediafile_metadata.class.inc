<?php
 /**
  * @file
  * Metadata functions.
  */
class mediamosa_asset_mediafile_metadata {

  /**
   * Properties.
   */
  const VIDEO_CODEC = 'video_codec';
  const VIDEO_CODEC_LENGTH = 50;
  const COLORSPACE = 'colorspace';
  const COLORSPACE_LENGTH = 10;
  const WIDTH = 'width';
  const HEIGHT = 'height';
  const FPS = 'fps';
  const FPS_LENGTH = 10;
  const AUDIO_CODEC = 'audio_codec';
  const AUDIO_CODEC_LENGTH = 50;
  const SAMPLE_RATE = 'sample_rate';
  const CHANNELS = 'channels';
  const CHANNELS_LENGTH = 10;
  const FILE_DURATION = 'file_duration';
  const FILE_DURATION_LENGTH = 11;
  const CONTAINER_TYPE = 'container_type';
  const CONTAINER_TYPE_LENGTH = 50;
  const BITRATE = 'bitrate';
  const BPP = 'bpp';
  const BPP_LENGTH = 5;
  const FILESIZE = 'filesize';
  const MD5 = 'md5';
  const MIME_TYPE = 'mime_type';
  const MIME_TYPE_LENGTH = 80;
  const IS_HINTED = 'is_hinted';
  const IS_HINTED_TRUE = 'TRUE';
  const IS_HINTED_FALSE = 'FALSE';
  const IS_INSERTED_MD = 'is_inserted_md';
  const IS_INSERTED_MD_TRUE = 'TRUE';
  const IS_INSERTED_MD_FALSE = 'FALSE';
  const STILL_TIME_CODE = 'still_time_code';
  const STILL_ORDER = 'still_order';
  const STILL_TYPE = 'still_type';
  const STILL_TYPE_NONE = 'NONE';
  const STILL_TYPE_NORMAL = 'NORMAL';
  const STILL_TYPE_SECOND = 'SECOND';
  const STILL_TYPE_SCENE = 'SCENE';
  const STILL_TYPE_PICTURE = 'PICTURE';
  const STILL_FORMAT = 'still_format';
  const STILL_FORMAT_LENGTH = 50;
  const STILL_DEFAULT = 'still_default';
  const STILL_DEFAULT_TRUE = 'TRUE';
  const STILL_DEFAULT_FALSE = 'FALSE';

  /**
   * Common video container types
   */
  const CONTAINER_TYPE_WMV = 'asf';
  const CONTAINER_TYPE_MP3 = 'mp3';
  const CONTAINER_TYPE_MP4 = 'mov;mp4;m4a;3gp;3g2;mj2';
  const CONTAINER_TYPE_FLV = 'flv';
  const CONTAINER_TYPE_OGG = 'ogg';
  const CONTAINER_TYPE_WEBM = 'webm';
  const CONTAINER_TYPE_MATROSKA_WEBM = 'matroska;webm';

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the mediafile metadata.
   *
   * @param string $mediafile_id
   * @fields array (array(prop_name, type)) The fields to give. If empty, then give all.
   */
  public static function get_with_mediafileid($mediafile_id, array $fields = array()) {
    $values = array();

    if (is_array($fields) && !empty($fields)) {
      foreach ($fields as $field) {
        // TODO: same prop_name with different types.
        $values[$field['prop_name']] = self::get_mediafile_metadata($mediafile_id, $field['prop_name'], $field['type']);
      }
    }
    else {
      // Get all.
      $values = self::get_all_mediafile_metadata($mediafile_id);
    }

    return $values;
  }

  /**
   * Test if mediafile can be played.
   *
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   *
   * @throws mediamosa_exception_error
   */
  public static function is_playable($mediafile_id) {
    // Get the mimetype.
    $mimetype = mediamosa_asset_mediafile::get_mime_type($mediafile_id);

    if ($mimetype == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_PLAY_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }

    if (empty($mimetype)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_PLAY_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile is not analysed.'));
    }
  }

  /**
   * Test if mediafile can be transcoded.
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   */
  public static function is_transcodable($mediafile_id) {
    $value = self::get_mediafile_metadata($mediafile_id, 'mime_type', mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    if ($value == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }

  /**
   * Delete all metadata properties of mediafile.
   *
   * @param string $mediafile_id
   */
  public static function delete_by_mediafileid($mediafile_id) {
    return self::delete_all_mediafile_metadata($mediafile_id);
  }

  /**
   * Calculate the bits per plane.
   *
   * @param $width
   * @param $height
   * @param $fps
   * @param $bitrate
   */
  static function calculate_bpp($width, $height, $fps, $bitrate) {
    $result = '';
    if ($width != '' && $height != '' && $fps != '' && $bitrate != '') {
      $result = round((($bitrate * 1000) / ($fps * $width * $height)), 2);
    }

    return $result;
  }

  /**
   * Fill the mediafile metadata.
   *
   * @param string $still_id
   *   The still ID.
   * @param integer $width
   *   The width of the still.
   * @param $height
   *   The height of the still.
   * @param integer $filesize
   *   The size of the file in bytes.
   * @param string $mime
   *   The content-type (mime-type).
   * @param $file_type
   * @param string $md5
   *   The MD5 of the file.
   */
  public static function create_metadata_still_default($still_id, $width, $height, $filesize, $mime, $file_type, $md5) {
    // Create mediafile metadata.
    $fields = array(
      array(
        'value' => $width,
        'prop_name' => mediamosa_asset_mediafile_metadata::WIDTH,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $height,
        'prop_name' => mediamosa_asset_mediafile_metadata::HEIGHT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $filesize,
        'prop_name' => mediamosa_asset_mediafile_metadata::FILESIZE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $md5,
        'prop_name' => mediamosa_asset_mediafile_metadata::MD5,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => $mime,
        'prop_name' => mediamosa_asset_mediafile_metadata::MIME_TYPE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => 0,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_ORDER,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $file_type,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_FORMAT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_TYPE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_FALSE,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
    );

    self::create_mediafile_multiple_metadata($still_id, $fields);
  }

  /**
   * Parse and store the returned data from the analyse.
   *
   * @param string mediafile_id
   *   The mediafile ID.
   * @param array $metadata
   *   The name-value pair array.
   *
   * @return string
   *   The ffmpeg output.
   */
  public static function store_metadata($mediafile_id, array $metadata) {
    // Remove current metadata.
    self::delete_by_mediafileid($mediafile_id);

    // Store basic information.
    $mediafile_uri = mediamosa_storage::get_uri_mediafile($mediafile_id);

    // Get the filesize.
    $filesize = mediamosa_io::filesize($mediafile_uri);

    // Get the md5.
    $md5 = mediamosa_io::md5_file($mediafile_uri);

    // Create filesize metadata.
    self::create_mediafile_metadata_int($mediafile_id, (int) $filesize, mediamosa_asset_mediafile_metadata::FILESIZE);

    // Create md5 metadata.
    self::create_mediafile_metadata_char($mediafile_id, $md5, mediamosa_asset_mediafile_metadata::MD5);

    // Store analyse.
    //
    // MIME-type.
    $value = isset($metadata['MIME-type']) ? $metadata['MIME-type'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::MIME_TYPE, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // Video-codec.
    $value = isset($metadata['Video-codec']) ? $metadata['Video-codec'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::VIDEO_CODEC, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // Video-colorspace.
    $value = isset($metadata['Video-colorspace']) ? $metadata['Video-colorspace'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::COLORSPACE, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // Video-size.
    list($width, $height) = isset($metadata['Video-size']) ? explode('x', $metadata['Video-size']) : array(0, 0);
    self::create_mediafile_metadata($mediafile_id, $width, mediamosa_asset_mediafile_metadata::WIDTH, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
    self::create_mediafile_metadata($mediafile_id, $height, mediamosa_asset_mediafile_metadata::HEIGHT, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);

    // Video-framespersecond.
    $fps = isset($metadata['Video-framespersecond']) ? $metadata['Video-framespersecond'] : '';
    self::create_mediafile_metadata($mediafile_id, $fps, mediamosa_asset_mediafile_metadata::FPS, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // Audio-codec.
    $value = isset($metadata['Audio-codec']) ? $metadata['Audio-codec'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::AUDIO_CODEC, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // Audio-frequency.
    $value = isset($metadata['Audio-frequency']) ? $metadata['Audio-frequency'] : '0';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::SAMPLE_RATE, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);

    // Audio-channels.
    $value = isset($metadata['Audio-channels']) ? $metadata['Audio-channels'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::CHANNELS, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // file-duration.
    $value = isset($metadata['File-duration']) ? $metadata['File-duration'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::FILE_DURATION, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // container.
    $value = isset($metadata['File-type']) ? $metadata['File-type'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // file-bitrate.
    $bitrate = isset($metadata['File-bitrate']) ? intval($metadata['File-bitrate']) : '0';
    if ($bitrate) {
      // Don't store invalid data.
      self::create_mediafile_metadata($mediafile_id, $bitrate, mediamosa_asset_mediafile_metadata::BITRATE, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
    }

    // bpp.
    $value = (string) self::calculate_bpp($width, $height, $fps, $bitrate);
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::BPP, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // is hinted.
    $value = isset($metadata['Is-hinted']) ? $metadata['Is-hinted'] : 'no';
    $value = strcasecmp($value, 'yes') == 0 ? 'TRUE' : 'FALSE';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::IS_HINTED, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // has inserted extra metadata.
    $value = isset($metadata['Is-inserted-md']) ? $metadata['Is-inserted-md'] : 'no';
    $value = strcasecmp($value, 'yes') == 0 ? 'TRUE' : 'FALSE';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::IS_INSERTED_MD, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    // The output of ffmpeg.
    $ffmpeg_output = isset($metadata['ffmpeg-output']) ? implode('\n', explode('}-{', $metadata['ffmpeg-output'])) : '';

    // Update statistics.
    mediamosa_statistics::update_job_transcode_filesize($mediafile_id, (int) $filesize);

    // Done.
    return $ffmpeg_output;
  }

  /**
   * Store the analyse result in the mediafile metadata.
   *
   * @param integer $job_id
   *   The job ID.
   * @param array $analyse_result
   *   The result of the analyse.
   */
  public static function store_analyse($job_id, array $analyse_result) {
    // Get the job.
    $job = mediamosa_job::get($job_id);

    // Get the mediafile_id.
    $mediafile_id = $job[mediamosa_job_db::MEDIAFILE_ID];

    // Parse and store metadata from the analyse result.
    $ffmpeg_output = self::store_metadata($mediafile_id, $analyse_result);

    // Store in error description.
    mediamosa_job::store_error_description($job_id, $ffmpeg_output);

    // Normalize to asset.
    mediamosa_asset::update_asset_info_with_mediafileid($mediafile_id);

    // Do we need to create an still job aswell?
    if ($job[mediamosa_job_db::CREATE_STILL] == mediamosa_job_db::CREATE_STILL_TRUE) {
      $still_parameters = (empty($job[mediamosa_job_db::STILL_PARAMETERS]) ? array() : unserialize($job[mediamosa_job_db::STILL_PARAMETERS]));

      try {
        mediamosa_job::create_job_still($job[mediamosa_job_db::APP_ID], $job[mediamosa_job_db::OWNER_ID], FALSE, $mediafile_id, $still_parameters);
      }
      catch (mediamosa_exception_error_still_is_not_creatable $e) {
        // The still is not creatable from the mediafile. Eg. it is an audio
        // file. Do nothing.
      }
    }
  }

  /**
   * Store the analyse result in the mediafile metadata without running or
   * during a job.
   *
   * @param array $analyse_result
   */
  public static function store_analyse_without_job($job_id, array $analyse_result, $mediafile_id) {
    // Parse and store metadata from the analyse result.
    $ffmpeg_output = self::store_metadata($mediafile_id, $analyse_result);

    // Store in error description.
    mediamosa_job::store_error_description($job_id, $ffmpeg_output);

    $job = mediamosa_job::get($job_id, array(mediamosa_job_db::MEDIAFILE_DEST));
    // Is it a retranscode job?
    if ($job[mediamosa_job_db::MEDIAFILE_DEST]) {
      // Get the realpath before we remove the mediafile from database.
      $mediafile_realpath_dest = mediamosa_storage::get_realpath_mediafile($job[mediamosa_job_db::MEDIAFILE_DEST]);

      // Delete the original so it can be overwritten.
      mediamosa_asset_mediafile::delete($job[mediamosa_job_db::MEDIAFILE_DEST]);

      // Move the file.
      mediamosa_io::move(
        mediamosa_storage::get_uri_mediafile($mediafile_id),
        $mediafile_realpath_dest
      );

      // Turn off foreign keys so we can move mediafile data and metadata.
      mediamosa_db::db_query('SET foreign_key_checks = 0');

      // Update mediamosa_asset_mediafile table.
      mediamosa_asset_mediafile::update_mediafile_id($mediafile_id, $job[mediamosa_job_db::MEDIAFILE_DEST]);

      // Update mediamosa_asset_mediafile_metadata table.
      self::move_mediafile_metadata($mediafile_id, $job[mediamosa_job_db::MEDIAFILE_DEST]);

      // And turn it back on.
      mediamosa_db::db_query('SET foreign_key_checks = 1');

      // Take the new mediafile_id.
      $mediafile_id = $job[mediamosa_job_db::MEDIAFILE_DEST];
    }

    // Normalize to asset.
    mediamosa_asset::update_asset_info_with_mediafileid($mediafile_id);
  }

  /**
   * Move the technical metadata from one mediafile to another.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $mediafile_id_dest
   *   The mediafile ID of the destination.
   */
  public static function move_mediafile_metadata($mediafile_id, $mediafile_id_dest) {
    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->fields(array(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => $mediafile_id_dest))
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }

  /**
   * Create mediafile metadata value.
   *
   * @param string $mediafile_id
   * @param string|integer $value
   *   The value of the metadata property.
   * @param $prop_name Property name
   *   The name of the property.
   * @param string $type
   *   Type of the property.
   *
   * @see mediamosa_asset_mediafile_metadata_property_db::TYPE_*.
   */
  public static function create_mediafile_metadata($mediafile_id, $value, $prop_name, $type) {
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($prop_name, $type);

    // Make sure it exists.
    mediamosa_asset_mediafile::must_exists_quick($mediafile_id);

    $fields = array();
    if ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR) {
      $fields[mediamosa_asset_mediafile_metadata_db::VAL_CHAR] = $value;
    }
    elseif ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_INT) {
      try {
        $fields[mediamosa_asset_mediafile_metadata_db::VAL_INT] = mediamosa_type::check($prop_name, mediamosa_sdk::TYPE_INT, $value);
      }
      catch (mediamosa_exception_error $e) {
        $fields[mediamosa_asset_mediafile_metadata_db::VAL_INT] = 0;
      }
    }
    elseif ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME) {
      try {
        $fields[mediamosa_asset_mediafile_metadata_db::VAL_DATETIME] = mediamosa_type::check($prop_name, mediamosa_sdk::TYPE_DATETIME, $value);
      }
      catch (mediamosa_exception_error $e) {
        return;
      }
    }
    else {
      // Shouldn't be here.
      throw new mediamosa_exception_program_error('Invalid mediafile metadata type (@type)!', array('@type' => $type));
    }

    // Either update or insert.
    mediamosa_db::db_merge(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->key(array(
        mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => $mediafile_id,
        mediamosa_asset_mediafile_metadata_db::PROP_ID => $prop_id,
      ))
      ->fields($fields)
      ->updateFields($fields)
      ->execute();
  }

  /**
   * Create mediafile metadata value - INT.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string|integer $value
   *   The value of the metadata.
   * @param string $prop_name
   *   The name of the Property.
   */
  public static function create_mediafile_metadata_int($mediafile_id, $value, $prop_name) {
    self::create_mediafile_metadata($mediafile_id, $value, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
  }

  /**
   * Create mediafile metadata value - CHAR.
   *
   * @param mediafile_id
   * @param value Value of the metadata
   * @param prop_name Property name
   */
  public static function create_mediafile_metadata_char($mediafile_id, $value, $prop_name) {
    self::create_mediafile_metadata($mediafile_id, $value, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
  }

  /**
   * Create mediafile metadata value - DATETIME.
   *
   * @param mediafile_id
   * @param value Value of the metadata
   * @param prop_name Property name
   */
  public static function create_mediafile_metadata_datetime($mediafile_id, $value, $prop_name) {
    self::create_mediafile_metadata($mediafile_id, $value, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME);
  }

  /**
   * Create multiple metadata for a mediafile.
   *
   * @param mediafile_id
   * @param metadatas An array (array(value => value, prop_name => prop_name, type => type))
   */
  public static function create_mediafile_multiple_metadata($mediafile_id, $metadatas) {
    if (is_array($metadatas) && !empty($metadatas)) {
      foreach ($metadatas as $metadata) {
        self::create_mediafile_metadata($mediafile_id, $metadata['value'], $metadata['prop_name'], $metadata['type']);
      }
    }
  }

  /**
   * Get mediafile metadata value.
   *
   * @param string mediafile_id
   *   Unique id for the mediafile
   * @param string prop_name
   *   Property name
   * @param string type
   *   Type of the property
   *
   * @return string|NULL
   *   Property value, or NULL when not found.
   *
   * @throws mediamosa_exception_program_error()
   */
  public static function get_mediafile_metadata($mediafile_id, $prop_name, $type) {
    // Cache for faster lookups.
    $mediafile_metadata = &drupal_static('mediamosa_asset_mediafile_metadata', array());

    // Is required.
    if (empty($mediafile_id) || empty($prop_name) || empty($type)) {
      throw new mediamosa_exception_program_error(
        'Missing required input in @func, on line @line in file @file',
        array(
          '@func' => __FUNCTION__,
          '@line' => __LINE__,
          '@file' => __FILE__,
        )
      );
    }

    // Check if metadata for this mediafile exists.
    if (!isset($mediafile_metadata[$mediafile_id])) {
      // Don't let it grow to much, pop the first from the array to keep mem in
      // check.
      if (count($mediafile_metadata) > 50) {
        array_shift($mediafile_metadata);
      }

      // Default empty.
      $mediafile_metadata[$mediafile_id] = array();

      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'amm');
      $query->join(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'ammp', 'ammp.' . mediamosa_asset_mediafile_metadata_property_db::ID . '=' . 'amm.' . mediamosa_asset_mediafile_metadata_db::PROP_ID);

      $metadatas = $query
        ->fields('ammp', array(
          mediamosa_asset_mediafile_metadata_property_db::NAME,
          mediamosa_asset_mediafile_metadata_property_db::TYPE,
        ))
        ->fields('amm', array(
          mediamosa_asset_mediafile_metadata_db::ID,
          mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
          mediamosa_asset_mediafile_metadata_db::VAL_INT,
          mediamosa_asset_mediafile_metadata_db::VAL_DATETIME,
        ))
        ->condition('amm.' . mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
        ->orderBy('amm.' . mediamosa_asset_mediafile_metadata_db::ID, 'ASC')
        ->execute();

      // Mapper type to val.
      $types2field = array(
        mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        mediamosa_asset_mediafile_metadata_property_db::TYPE_INT => mediamosa_asset_mediafile_metadata_db::VAL_INT,
        mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME => mediamosa_asset_mediafile_metadata_db::VAL_DATETIME,
      );
      // Map as [mediafile_id][name][type] = value.
      foreach ($metadatas as $metadata) {
        $mediafile_metadata[$mediafile_id][$metadata[mediamosa_asset_mediafile_metadata_property_db::NAME]][$metadata[mediamosa_asset_mediafile_metadata_property_db::TYPE]] = $metadata[$types2field[$metadata[mediamosa_asset_mediafile_metadata_property_db::TYPE]]];
      }
    }

    // No value?
    if (empty($mediafile_metadata[$mediafile_id][$prop_name][$type])) {
      return;
    }

    // Propname ok, but wrong type?
    if (isset($mediafile_metadata[$mediafile_id][$prop_name]) && !isset($mediafile_metadata[$mediafile_id][$prop_name][$type])) {
      mediamosa_debug::log($prop_name);
      mediamosa_watchdog::log_export($mediafile_metadata[$mediafile_id]);
      throw new mediamosa_exception_program_error('Unknown metadata property type; @type', array('@type' => $type));
    }

    return $mediafile_metadata[$mediafile_id][$prop_name][$type];
  }

  /**
   * Get mediafile metadata value - INT.
   *
   * @param mediafile_id
   *   Unique id for the mediafile.
   * @param prop_name
   *   Property name.
   *
   * @return
   *   Property value, or NULL.
   */
  public static function get_mediafile_metadata_int($mediafile_id, $prop_name) {
    return self::get_mediafile_metadata($mediafile_id, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
  }

  /**
   * Get mediafile metadata value - CHAR.
   *
   * @param mediafile_id
   *   Unique id for the mediafile.
   * @param prop_name
   *   Property name.
   *
   * @return
   *   Property value, or NULL.
   */
  public static function get_mediafile_metadata_char($mediafile_id, $prop_name) {
    return self::get_mediafile_metadata($mediafile_id, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
  }

  /**
   * Get mediafile metadata value - DATETIME.
   *
   * @param mediafile_id
   *   Unique id for the mediafile
   * @param prop_name
   *   Property name
   *
   * @return
   *   Property value, or NULL.
   */
  public static function get_mediafile_metadata_datetime($mediafile_id, $prop_name) {
    return self::get_mediafile_metadata($mediafile_id, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME);
  }

  /**
   * Get all the technical metadata values of mediafile.
   *
   * @todo: Get rid of the calls to get_property by joining in the first query
   *        with the property table.
   *
   *        Make it work with get_mediafile_metadata().
   *
   * @param mediafile_id
   *   Unique id for the mediafile.
   *
   * @return
   *   Value of the properties in an associative array.
   */
  public static function get_all_mediafile_metadata($mediafile_id) {
    $values = array();

    $result = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mm')
      ->fields('mm', array(
        mediamosa_asset_mediafile_metadata_db::PROP_ID,
        mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        mediamosa_asset_mediafile_metadata_db::VAL_INT,
        mediamosa_asset_mediafile_metadata_db::VAL_DATETIME,
      ))
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();

    // Hotfix for cache.
    $reset_cache = TRUE;
    foreach ($result as $row) {
      $prop = mediamosa_asset_mediafile_metadata_property::get_property($row[mediamosa_asset_mediafile_metadata_db::PROP_ID], $reset_cache);
      $reset_cache = FALSE;

      // TODO: same prop_name with different types.
      if ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR) {
        $values[$prop['prop_name']] = $row[mediamosa_asset_mediafile_metadata_db::VAL_CHAR];
      }
      elseif ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_INT) {
        $values[$prop['prop_name']] = $row[mediamosa_asset_mediafile_metadata_db::VAL_INT];
      }
      elseif ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME) {
        $values[$prop['prop_name']] = $row[mediamosa_asset_mediafile_metadata_db::VAL_DATETIME];
      }
      else {
        // Shouldn't be here.
        assert(0);
      }
    }

    return $values;
  }

  /**
   * Delete mediafile metadata value.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $prop_name
   *   The property name.
   * @param string Type of the property
   *
   * @return integer
   *   The number of deleted records or FALSE.
   */
  public static function delete_mediafile_metadata($mediafile_id, $prop_name, $type) {
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($prop_name, $type);
    if (!$prop_id) {
      return FALSE;
    }

    // Reset static cache.
    $mediafile_metadata = &drupal_static('mediamosa_asset_mediafile_metadata', array());
    unset($mediafile_metadata[$mediafile_id]);

    return mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id)
      ->execute();
  }

  /**
   * Delete all the metadata value of mediafile.
   *
   * @param string $mediafile_id
   *   The MediaFile ID to remove metadata from.
   *
   * @return integer
   *   The number of metadata properties deleted.
   */
  public static function delete_all_mediafile_metadata($mediafile_id) {
    // Reset static cache.
    $mediafile_metadata = &drupal_static('mediamosa_asset_mediafile_metadata', array());
    unset($mediafile_metadata[$mediafile_id]);

    return mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }

  /**
   * Get all the technical metadata values of asset.
   *
   * @param asset_id
   *   The asset ID.
   *
   * @return string
   *   Values of the properties in an associative array.
   */
  public static function get_all_mediafile_metadata_for_asset($asset_id) {
    // Result.
    $values = array();

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
    $result = $query
      ->fields('mf')
      ->fields('mfm', array(
        mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
        mediamosa_asset_mediafile_metadata_db::PROP_ID,
        mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        mediamosa_asset_mediafile_metadata_db::VAL_INT,
        mediamosa_asset_mediafile_metadata_db::VAL_DATETIME,
      ))
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->execute();

    foreach ($result as $row) {
      if (empty($row[mediamosa_asset_mediafile_metadata_db::PROP_ID])) {
        continue;
      }

      // Get property.
      $prop = mediamosa_asset_mediafile_metadata_property::get_property($row[mediamosa_asset_mediafile_metadata_db::PROP_ID], TRUE);

      // Store.
      $values[$row[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID]]['metadata'][$prop['prop_name']] = $row[$prop['type']];

      // Get mediafile.
      $values[$row[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID]]['mediafile'] = $row;
    }

    return $values;
  }
}
