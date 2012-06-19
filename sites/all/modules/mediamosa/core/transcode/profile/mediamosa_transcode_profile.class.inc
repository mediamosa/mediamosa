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
  * transcode_profile module.
  */

class mediamosa_transcode_profile {

  /**
   * Get transcode profile for app.
   *
   * @param integer $transcode_profile_id
   *  Profile to get.
   * @param integer $app_id
   *  Application ID.
   */
  public static function get($transcode_profile_id, $app_id = NULL) {
    $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')
      ->fields('tp')
      ->condition(mediamosa_transcode_profile_db::ID, $transcode_profile_id);

    if (isset($app_id)) {
      $query->condition(mediamosa_transcode_profile_db::APP_ID, array(0, $app_id), 'IN');
    }

    // Do the query.
    return $query->execute()->fetchAssoc();
  }

  /**
   * Get the default transcode profile.
   *
   * @param integer $app_id
   *  Application ID.
   *
   */
  public static function get_default($app_id) {
    return mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')
      ->fields('tp')
      ->condition(mediamosa_transcode_profile_db::APP_ID, array(0, $app_id), 'IN')
      ->condition(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Retrieve a listing of transcode profiles.
   *
   * @param $fields
   */
  public static function get_all(array $fields = array()) {

    // Build the query.
    $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp');

    if (count($fields)) {
      foreach ($fields as $key => $field) {
        $query->addField('tp', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('tp');
    }

    return $query->execute()->fetchAll();

  }

  /**
   * Get listing transcode profiles for app.
   *
   * @param integer $app_id
   *  Application ID.
   */
  public static function search($app_id) {

    return mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')
      ->fields('tp')
      ->condition(mediamosa_transcode_profile_db::APP_ID, array(0, $app_id), 'IN')
      ->execute();
  }

  /**
   * Get transcode profile for app by name.
   *
   * @param integer $app_id
   *  Application ID.
   */
  public static function get_by_name($app_id, $profile_name) {

    return mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')
      ->fields('tp')
      ->condition(mediamosa_transcode_profile_db::APP_ID, array(0, $app_id), 'IN')
      ->condition(mediamosa_transcode_profile_db::PROFILE, $profile_name)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Retrieve the transcode profiles based on tool.
   * @param $tool
   */
  public static function get_by_tool($app_id, $tool, $nice_parameter = NULL) {

    $query = mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'mtm')
      ->fields('mtm')
      ->condition(mediamosa_tool_params_db::TOOL, $tool);

    // If no App ids are given, then return all.
    if (!empty($app_id)) {
      $query->condition(mediamosa_transcode_profile_db::APP_ID, array(0, $app_id), 'IN');
    }

    // Nice parameter.
    if (!empty($nice_parameter)) {
      $query->condition(mediamosa_tool_params_db::NICE_PARAMETER, $nice_parameter);
    }

    // Return the result.
    return $query->execute();
  }

  /**
   * Will test if transcode profile exits, will throw when fail.
   *
   * @param integer profile_id
   */
  public static function must_exists($app_id, $profile_id) {
    return mediamosa_db::db_must_exists(mediamosa_transcode_profile_db::TABLE_NAME, array(mediamosa_transcode_profile_db::APP_ID => $app_id, mediamosa_transcode_profile_db::ID => $profile_id));
  }

  /**
   * Create a transcode profile.
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $name
   *  The name of the profile.
   * @param integer $version
   *  Version number.
   * @param bool $default
   *  Is default.
   * @param string $tool
   *  Tool used.
   * @param string $file_extension
   *  Extension.
   * @param string $command
   *  Command.
   */
  public static function create($app_id, $name, $version, $default, $tool, $file_extension, $command) {

    // Make sure name is not taken.
    if (mediamosa_transcode_profile::get_by_name($app_id, $name)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TRANSCODE_PROFILE_EXISTS, array('@profile_name' => $name));
    }

    // When setting default, make sure no other default is set.
    if ($default) {
      $a_fields = array(
        mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE
      );

      // Add changed value.
      $a_fields = mediamosa_db::db_update_enrich($a_fields);

      mediamosa_db::db_update(mediamosa_transcode_profile_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_transcode_profile_db::APP_ID, $app_id)
        ->execute();
    }

    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE, $name);

    $node->{mediamosa_transcode_profile_db::APP_ID} = $app_id;
    $node->{mediamosa_transcode_profile_db::VERSION} = $version;
    $node->{mediamosa_transcode_profile_db::PROFILE} = $name;
    $node->{mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE} = $default ? mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE : mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE;
    $node->{mediamosa_transcode_profile_db::TOOL} = $tool;
    $node->{mediamosa_transcode_profile_db::FILE_EXTENSION} = $file_extension;
    $node->{mediamosa_transcode_profile_db::COMMAND} = $command;

    node_save($node);

    // Get a fresh $node object with other parameters (eg. transcode_profile_id).
    $node = node_load($node->nid);

    // Return the profile id.
    return $node->transcode_profile_id;
  }

  /**
   * Update a transcode profile.
   * FIXME: 2.x port: updating the app_id ?
   *
   * @param integer $app_id
   *  The application ID.
   * @param integer $profile_id
   *  The ID of profile to update.
   * @param string $name
   *  The name of the profile.
   * @param integer $version
   *  Version number.
   * @param bool $default
   *  Is default.
   * @param string $tool
   *  Tool used.
   * @param string $file_extension
   *  Extension.
   * @param string $command
   *  Command.
   */
  public static function update($app_id, $profile_id, $name = NULL, $version = NULL, $default = 'FALSE', $tool = NULL, $file_extension = NULL, $command = NULL) {

    // Must exists.
    mediamosa_transcode_profile::must_exists($app_id, $profile_id);

    // When setting default, make sure no other default is set.
    if ($default) {
      $a_fields = array(
        mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE
      );

      // Add changed value.
      $a_fields = mediamosa_db::db_update_enrich($a_fields);

      // Update.
      mediamosa_db::db_update(mediamosa_transcode_profile_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_transcode_profile_db::APP_ID, $app_id)
        ->execute();
    }

    // Required parameters
    $a_fields = array(
      mediamosa_transcode_profile_db::APP_ID => $app_id,
    );
    // Optional parameters
    if (!is_null($name)) {
      $a_fields[mediamosa_transcode_profile_db::PROFILE] = $name;
    }
    if (!is_null($version)) {
      $a_fields[mediamosa_transcode_profile_db::VERSION] = $version;
    }
    $a_fields[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE] = $default ?
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE :
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE;
    if (!is_null($tool)) {
      $a_fields[mediamosa_transcode_profile_db::TOOL] = $tool;
    }
    if (!is_null($file_extension)) {
      $a_fields[mediamosa_transcode_profile_db::FILE_EXTENSION] = $file_extension;
    }
    if (!is_null($command)) {
      $a_fields[mediamosa_transcode_profile_db::COMMAND] = $command;
    }

    // Add changed value.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    return mediamosa_db::db_update(mediamosa_transcode_profile_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_transcode_profile_db::ID, $profile_id)
      ->execute();
  }

  /**
   * Delete a transcode profile.
   *
   * @param integer $app_id
   *  Application ID.
   * @param integer $profile_id
   *  The ID of the profile to delete.
   */
  public static function delete($app_id, $profile_id) {
    mediamosa_db::db_delete(mediamosa_transcode_profile_db::TABLE_NAME)
      ->condition(mediamosa_transcode_profile_db::ID, $profile_id)
      ->condition(mediamosa_transcode_profile_db::APP_ID, $app_id)
      ->execute();
  }

  /**
   * Go through the paramters and check input.
   *
   * 1.x: _vpx_jobs_check_command_parameters
   *
   * @param string $tool
   *  Tool to check.
   * @param string $command
   *  Command to check.
   */
  public static function check_command_parameters($tool, $command) {

    // Split the command into an array.
    $a_parameters = mediamosa_lib::create_named_array($command, ';', ':');

    // Check the values of the mapping.
    foreach ($a_parameters as $nice_parameter => $value) {
      mediamosa_tool_params::check_mapping($tool, $nice_parameter, $value);
    }
  }
}
