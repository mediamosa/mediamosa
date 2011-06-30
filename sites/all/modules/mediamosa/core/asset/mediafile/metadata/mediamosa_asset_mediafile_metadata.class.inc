<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

 /**
  * @file
  * Metadata functions.
  */
class mediamosa_asset_mediafile_metadata {

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

  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the mediafile metadata.
   *
   * @param string $mediafile_id
   * @fields array (array(prop_name, type)) The fields to give. If empty, then give all.
   */
  static public function get_with_mediafileid($mediafile_id, array $fields = array()) {
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
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   */
  static public function is_playable($mediafile_id) { // media_management_is_playable.
    $value = self::get_mediafile_metadata($mediafile_id, 'mime_type', mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

    if ($value == mediamosa_lib::MIME_TYPE_APPLICATION_X_EMPTY) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_PLAY_MEDIAFILE, array('@mediafile_id' => $mediafile_id, '@reason' => 'this mediafile has an empty filesize.'));
    }
  }

  /**
   * Test if mediafile can be transcoded.
   * Will throw an exception when failure.
   *
   * @param string $mediafile_id
   */
  static public function is_transcodable($mediafile_id) {
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
  static public function delete_by_mediafileid($mediafile_id) {
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
  function calculate_bpp($width, $height, $fps, $bitrate) {
    $result = '';
    if ($width != '' && $height != '' && $fps != '' && $bitrate != '') {
      $result = round((($bitrate * 1000) / ($fps * $width * $height)), 2);
    }

    return $result;
  }

  /**
   * Fill the mediafile metadata.
   *
   * @param $still_id
   * @param $width
   * @param $height
   * @param $filesize
   * @param $mime
   * @param $file_type
   */
  static public function create_metadata_still_default($still_id, $width, $height, $filesize, $mime, $file_type) {
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
      //mediamosa_asset_mediafile_metadata_db::BPP => '',
      array(
        'value' => $filesize,
        'prop_name' => mediamosa_asset_mediafile_metadata::FILESIZE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
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
   * Parse and store the returned string from analyse.
   *
   * 1.x: _vpx_jobhandler_parse_metadata
   *
   * @param mediafile_id
   * @param string $analyse_result
   */
  static public function parse_store_metadata($mediafile_id, $analyse_result) {
    // Basic values.
    $type_int = mediamosa_asset_mediafile_metadata_property_db::TYPE_INT;
    $type_char = mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR;
    $type_datetime = mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME;

    $a_metadata = mediamosa_lib::create_named_array($analyse_result, "\n", ': ');

    // MIME-type.
    $value = isset($a_metadata['MIME-type']) ? $a_metadata['MIME-type'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::MIME_TYPE, $type_char);

    // Video-codec.
    $value = isset($a_metadata['Video-codec']) ? $a_metadata['Video-codec'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::VIDEO_CODEC, $type_char);

    // Video-colorspace.
    $value = isset($a_metadata['Video-colorspace']) ? $a_metadata['Video-colorspace'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::COLORSPACE, $type_char);

    // Video-size.
    list($width, $height) = isset($a_metadata['Video-size']) ? explode('x', $a_metadata['Video-size']) : array(0, 0);
    self::create_mediafile_metadata($mediafile_id, $width, mediamosa_asset_mediafile_metadata::WIDTH, $type_int);
    self::create_mediafile_metadata($mediafile_id, $height, mediamosa_asset_mediafile_metadata::HEIGHT, $type_int);

    // Video-framespersecond.
    $fps = isset($a_metadata['Video-framespersecond']) ? $a_metadata['Video-framespersecond'] : '';
    self::create_mediafile_metadata($mediafile_id, $fps, mediamosa_asset_mediafile_metadata::FPS, $type_char);

    // Audio-codec.
    $value = isset($a_metadata['Audio-codec']) ? $a_metadata['Audio-codec'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::AUDIO_CODEC, $type_char);

    // Audio-frequency.
    $value = isset($a_metadata['Audio-frequency']) ? $a_metadata['Audio-frequency'] : '0';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::SAMPLE_RATE, $type_int);

    // Audio-channels.
    $value = isset($a_metadata['Audio-channels']) ? $a_metadata['Audio-channels'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::CHANNELS, $type_char);

    // file-duration.
    $value = isset($a_metadata['File-duration']) ? $a_metadata['File-duration'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::FILE_DURATION, $type_char);

    // container.
    $value = isset($a_metadata['File-type']) ? $a_metadata['File-type'] : '';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE, $type_char);

    // file-bitrate.
    $bitrate = isset($a_metadata['File-bitrate']) ? intval($a_metadata['File-bitrate']) : '0';
    self::create_mediafile_metadata($mediafile_id, $bitrate, mediamosa_asset_mediafile_metadata::BITRATE, $type_int);

    // bpp.
    $value = (string)self::calculate_bpp($width, $height, $fps, $bitrate);
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::BPP, $type_char);

    // is hinted.
    $value = isset($a_metadata['Is-hinted']) ? $a_metadata['Is-hinted'] : 'no';
    $value = strcasecmp($value, 'yes') == 0 ? 'TRUE' : 'FALSE';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::IS_HINTED, $type_char);

    // has inserted extra metadata.
    $value = isset($a_metadata['Is-inserted-md']) ? $a_metadata['Is-inserted-md'] : 'no';
    $value = strcasecmp($value, 'yes') == 0 ? 'TRUE' : 'FALSE';
    self::create_mediafile_metadata($mediafile_id, $value, mediamosa_asset_mediafile_metadata::IS_INSERTED_MD, $type_char);

    // The output of ffmpeg.
    $ffmpeg_output = isset($a_metadata['ffmpeg-output']) ? implode('\n', explode('}-{', $a_metadata['ffmpeg-output'])) : '';

    // Done.
    return $ffmpeg_output;
  }

  /**
   * Store the analyse result in the mediafile metadata.
   *
   * 1.x: _vpx_jobhandler_update_analyse_job_info_to_mediafile
   *
   * @param $job_id
   * @param $analyse_result
   */
  static public function store_analyse($job_id, $analyse_result) {
    // Get the job.
    $job = mediamosa_job::get($job_id);

    // Get the filesize.
    $filesize = mediamosa_io::filesize(mediamosa_configuration_storage::mediafile_id_filename_get($job[mediamosa_job_db::MEDIAFILE_ID]));

    $mediafile_id = $job[mediamosa_job_db::MEDIAFILE_ID];

    // We don't remove any previous metadatas.
    //self::delete_all_mediafile_metadata($mediafile_id);

    // Parse and store metadata from the analyse result.
    $ffmpeg_output = self::parse_store_metadata($mediafile_id, implode("\n", $analyse_result));

    // Create filesize metadata.
    self::create_mediafile_metadata($mediafile_id, (int)$filesize, mediamosa_asset_mediafile_metadata::FILESIZE, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);

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
        // The still is not creatable from the mediafile. Eg. it is an audio file.
        // Do nothing.
      }
    }
  }

  /**
   * Store the analyse result in the mediafile metadata without running or
   * during a job.
   *
   * @param $analyse_result
   */
  static public function store_analyse_without_job($job_id, $analyse_result, $mediafile_id) {
    // Get the filesize.
    $filesize = mediamosa_io::filesize(mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id));

    // Parse and store metadata from the analyse result.
    $ffmpeg_output = self::parse_store_metadata($mediafile_id, implode("\n", $analyse_result));

    // Create filesize metadata.
    self::create_mediafile_metadata($mediafile_id, (int) $filesize, mediamosa_asset_mediafile_metadata::FILESIZE, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);

    // Store in error description.
    mediamosa_job::store_error_description($job_id, $ffmpeg_output);

    $job = mediamosa_job::get($job_id, array(mediamosa_job_db::MEDIAFILE_DEST));
    // Is it a retranscode job?
    if ($job[mediamosa_job_db::MEDIAFILE_DEST]) {
      $mediafile_dest = $job[mediamosa_job_db::MEDIAFILE_DEST];
      // Delete the original.
      mediamosa_asset_mediafile::delete($mediafile_dest);
      // Move the file.
      mediamosa_io::move(
        mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id),
        mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_dest)
      );

      mediamosa_db::db_query("SET foreign_key_checks = 0");
      // Update mediamosa_asset_mediafile table.
      mediamosa_asset_mediafile::update_mediafile_id($mediafile_id, $mediafile_dest);
      // Update mediamosa_asset_mediafile_metadata table.
      self::update_mediafile_id($mediafile_id, $mediafile_dest);
      mediamosa_db::db_query("SET foreign_key_checks = 1");

      // Update the mediafile_id.
      $mediafile_id = $mediafile_dest;
    }

    // Normalize to asset.
    mediamosa_asset::update_asset_info_with_mediafileid($mediafile_id);

    // Update statistics
    mediamosa_statistics::update_job_transcode_filesize($mediafile_id, (int) $filesize);
  }

  /**
   * Update the mediafile_id.
   *
   * @param mediafile_id
   * @param mediafile_dest
   */
  static public function update_mediafile_id($mediafile_id, $mediafile_dest) {
    $fields = array(
      mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => $mediafile_dest,
    );
    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }

  /**
   * Create mediafile metadata value.
   *
   * @param mediafile_id
   * @param value Value of the metadata
   * @param prop_name Property name
   * @param type Type of the property
   */
  static public function create_mediafile_metadata($mediafile_id, $value, $prop_name, $type) {
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($prop_name, $type);

    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

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

    $num_updated = mediamosa_db::db_update(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id)
      ->execute();

    if (!$num_updated) {
      $fields[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID] = $mediafile_id;
      $fields[mediamosa_asset_mediafile_metadata_db::PROP_ID] = $prop_id;

      $metadata_id = mediamosa_db::db_insert(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
        ->fields($fields)
        ->execute();
    }
  }

  /**
   * Create mediafile metadata value - INT.
   *
   * @param mediafile_id
   * @param value Value of the metadata
   * @param prop_name Property name
   */
  static public function create_mediafile_metadata_int($mediafile_id, $value, $prop_name) {
    self::create_mediafile_metadata($mediafile_id, $value, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
  }

  /**
   * Create mediafile metadata value - CHAR.
   *
   * @param mediafile_id
   * @param value Value of the metadata
   * @param prop_name Property name
   */
  static public function create_mediafile_metadata_char($mediafile_id, $value, $prop_name) {
    self::create_mediafile_metadata($mediafile_id, $value, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
  }

  /**
   * Create mediafile metadata value - DATETIME.
   *
   * @param mediafile_id
   * @param value Value of the metadata
   * @param prop_name Property name
   */
  static public function create_mediafile_metadata_datetime($mediafile_id, $value, $prop_name) {
    self::create_mediafile_metadata($mediafile_id, $value, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME);
  }

  /**
   * Create multiple metadata for a mediafile.
   *
   * @param mediafile_id
   * @param metadatas An array (array(value => value, prop_name => prop_name, type => type))
   */
  static public function create_mediafile_multiple_metadata($mediafile_id, $metadatas) {
    if (is_array($metadatas) && !empty($metadatas)) {
      foreach ($metadatas as $metadata) {
        self::create_mediafile_metadata($mediafile_id, $metadata['value'], $metadata['prop_name'], $metadata['type']);
      }
    }
  }

  /**
   * Get mediafile metadata value.
   *
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @param type Type of the property
   * @return Property value, or FALSE in case of validation error
   */
  static public function get_mediafile_metadata($mediafile_id, $prop_name, $type) {
    $value = NULL;

    if (!$mediafile_id || !$prop_name || !$type) {
      return $value;
    }

    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($prop_name, $type);
    if (!$prop_id) {
      return FALSE;
    }

    $result = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'amm')
      ->fields('amm', array(
        mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        mediamosa_asset_mediafile_metadata_db::VAL_INT,
        mediamosa_asset_mediafile_metadata_db::VAL_DATETIME,
      ))
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id)
      ->execute();

    foreach ($result as $row) {
      if ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR) {
        $value = $row[mediamosa_asset_mediafile_metadata_db::VAL_CHAR];
      }
      elseif ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_INT) {
        $value = $row[mediamosa_asset_mediafile_metadata_db::VAL_INT];
      }
      elseif ($type == mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME) {
        $value = $row[mediamosa_asset_mediafile_metadata_db::VAL_DATETIME];
      }
      else {
        // Shouldn't be here.
        assert(0);
      }
    }

    return $value;
  }

  /**
   * Get mediafile metadata value - INT.
   *
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @return Property value, or FALSE in case of validation error
   */
  static public function get_mediafile_metadata_int($mediafile_id, $prop_name) {
    return self::get_mediafile_metadata($mediafile_id, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
  }

  /**
   * Get mediafile metadata value - CHAR.
   *
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @return Property value, or FALSE in case of validation error
   */
  static public function get_mediafile_metadata_char($mediafile_id, $prop_name) {
    return self::get_mediafile_metadata($mediafile_id, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
  }

  /**
   * Get mediafile metadata value - DATETIME.
   *
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @return Property value, or FALSE in case of validation error
   */
  static public function get_mediafile_metadata_datetime($mediafile_id, $prop_name) {
    return self::get_mediafile_metadata($mediafile_id, $prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME);
  }

  /**
   * Get all the metadata values of mediafile.
   *
   * @param mediafile_id Unique id for the mediafile
   * @return Value of the properties in an associative array
   */
  static public function get_all_mediafile_metadata($mediafile_id) {
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

    foreach ($result as $row) {
      $prop = mediamosa_asset_mediafile_metadata_property::get_property($row[mediamosa_asset_mediafile_metadata_db::PROP_ID], TRUE);

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
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @param type Type of the property
   * @return Number of deleted records, or FALSE in case of validation error
   */
  static public function delete_mediafile_metadata($mediafile_id, $prop_name, $type) {
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($prop_name, $type);
    if (!$prop_id) {
      return FALSE;
    }

    $num_deleted = mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id)
      ->execute();

    return $num_deleted;
  }

  /**
   * Delete all the metadata value of mediafile.
   * @param mediafile_id Unique id for the mediafile
   */
  static public function delete_all_mediafile_metadata($mediafile_id) {
    $num_deleted = mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();

    return $num_deleted;
  }

  /**
   * Get all the technical metadata values of asset.
   *
   * @param asset_id
   *  The asset.
   *
   * @return
   *  Value of the properties in an associative array.
   */
  static public function get_all_mediafile_metadata_for_asset($asset_id) {
    $values = array();

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
    $query->fields('mf');
    $result = $query->fields('mfm', array(
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

      $prop = mediamosa_asset_mediafile_metadata_property::get_property($row[mediamosa_asset_mediafile_metadata_db::PROP_ID], TRUE);

      // TODO: same prop_name with different types.
      if ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR) {
        $values[$row[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID]]['metadata'][$prop['prop_name']] = $row[mediamosa_asset_mediafile_metadata_db::VAL_CHAR];
      }
      elseif ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_INT) {
        $values[$row[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID]]['metadata'][$prop['prop_name']] = $row[mediamosa_asset_mediafile_metadata_db::VAL_INT];
      }
      elseif ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME) {
        $values[$row[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID]]['metadata'][$prop['prop_name']] = $row[mediamosa_asset_mediafile_metadata_db::VAL_DATETIME];
      }
      else {
        // Shouldn't be here.
        assert(0);
      }

      // Get mediafile.
      $values[$row[mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID]]['mediafile'] = $row;
    }

    return $values;
  }


}
