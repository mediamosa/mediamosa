<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Tool metadata mapping functions.
  */

class mediamosa_tool_mapping {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get tool mapping with tool.
   *
   * @param string $tool
   * @param array $fields
   */
  public static function get($tool, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_tool_mapping_db::TABLE_NAME, 'tm');
    $query
      ->condition('tm.' . mediamosa_tool_mapping_db::TOOL, $tool)
      ->fields('tm', $fields);
    $result = $query->execute();

    return $result;
  }

  /**
   * Get all tool mapping.
   *
   * @param array $fields
   */
  public static function get_all(array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_tool_mapping_db::TABLE_NAME, 'tm');
    $query
      ->fields('tm', $fields);
    $result = $query->execute();

    return $result;
  }

  /**
   * Get asset metadata in special output format for tool mapping.
   */
  public static function get_asset_metadata_display_name($prop_id) {
    $metadata_property = mediamosa_asset_metadata_property::property_get_row($prop_id);
    $metadata_property_group = mediamosa_asset_metadata_property_group::property_group_row_get($metadata_property[mediamosa_asset_metadata_property_db::PROPGROUP_ID]);

    $group = $metadata_property_group[mediamosa_asset_metadata_property_group_db::NAME];
    switch ($metadata_property_group[mediamosa_asset_metadata_property_group_db::NAME]) {
      case 'dublin_core':
        $group = 'dc';
        break;
      case 'qualified_dublin_core':
        $group = 'qdc';
        break;
    }

    $asset_metadata = $group . '.' . $metadata_property[mediamosa_asset_metadata_property_db::NAME];

    return $asset_metadata;
  }

  /**
   * Analyse function for tools.
   * This function is good for analyse data in the following format:
   * "Something1   :   some value1"
   * "Something2   :   some value2"
   * ":" is the $separator.
   *
   * @param array $job_ext
   *   Job row.
   * @param sting $command_type
   *   Typo of the command for watchdog.
   * @param string $tool_id
   *   Tool unique ID.
   * @param string $execution_command
   *   The command for execution with place holders.
   *   Eg. 'pdfinfo @mediafile_location'
   * @param string $separator
   *   The separator for the command.
   */
  public static function tool_analyse($job_ext, $command_type, $tool_id, $execution_command, $separator) {
    foreach (mediamosa_tool_mapping::get($tool_id) as $record) {
      $property = mediamosa_asset_metadata_property::property_get_row($record[mediamosa_tool_mapping_db::PROP_ID]);

      $mapping_array[$record[mediamosa_tool_mapping_db::PARAMETER]] = $property[mediamosa_asset_metadata_property_db::NAME];
      $type_array[$record[mediamosa_tool_mapping_db::PARAMETER]] = $property[mediamosa_asset_metadata_property_db::TYPE];
    }

    $app_id = $job_ext[mediamosa_job_db::APP_ID];
    $app = mediamosa_app::get_by_appid($app_id);
    $job_id = $job_ext[mediamosa_job_db::ID];
    $mediafile_id = $job_ext[mediamosa_job_db::MEDIAFILE_ID];
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);
    $asset_id = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

    // Execute.
    $execution_string = strtr($execution_command, array(
      '@mediafile_location' => mediamosa_storage::get_uri_mediafile($mediafile),
    ));
    $output_array = array();
    $output_string = mediamosa_io::exec($execution_string, $output_array);
    $link_asset = mediamosa_job_server::get_asset_link($job_id);
    mediamosa_watchdog::log_mediafile($mediafile_id, 'Command for @command_type: @command ; returned output: @s_output - @a_output<br /><br />@link',
      array(
        '@command_type' => $command_type,
        '@command' => $execution_string,
        '@s_output' => $output_string,
        '@a_output' => print_r($output_array, TRUE),
        '@link' => $link_asset,
      )
    );

    // Parse.
    $parse_array = array();
    // Get full definitions.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id);
    $metadata_found = array();

    foreach ($output_array as $output_line) {
      if (strpos($output_line, $separator) !== FALSE) {
        // Map Asset metadata.
        // Eg. CreationDate:   Wed Jul  8 16:14:29 2009
        // => array('CreationDate', 'Wed Jul  8 16:14:29 2009')
        list($key, $metadata_value) = explode($separator, $output_line, 2);
        $key = drupal_strtolower(trim($key));
        $metadata_value = trim($metadata_value);

        if ($metadata_value && isset($mapping_array[$key]) && $mapping_array[$key]) {
          // Datetime conversion.
          if ($type_array[$key] == mediamosa_asset_metadata_property_db::TYPE_DATETIME) {
            $metadata_value = mediamosa_datetime::time_string2utc_date($metadata_value, $app[mediamosa_app_db::TIMEZONE]);
          }

          $parse_array[$mapping_array[$key]] = array($metadata_value);
        }

        $metadata_found[$key] = $metadata_value;
      }
    }

    // See if we can store some technical metadata.
    $visit_func = "mediamosa_tool_" . $tool_id . "_mediamosa_tool_store_technical_metadata";
    if (function_exists($visit_func)) {
      $technical_metadata_mapping = call_user_func($visit_func, $metadata_found);
    }

    if ($technical_metadata_mapping) {
      foreach ($technical_metadata_mapping as $key => $value) {
        mediamosa_asset_mediafile_metadata::delete_mediafile_metadata($mediafile_id, $key, key($value));
        mediamosa_asset_mediafile_metadata::create_mediafile_multiple_metadata($mediafile_id, array( array(
         'value' => current($value),
         'prop_name' => $key,
         'type' => key($value))));
      }
    }

    // Store found asset metadata.
    mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, $parse_array, 'update');
  }
}
