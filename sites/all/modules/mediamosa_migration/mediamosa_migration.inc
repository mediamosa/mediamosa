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
  * Migration functions.
  */
/*
 * Taken from sites/default/settings.php ;

// Migration memo 1.0 databases;
$databases['mig_memo']['default'] = array(
  'driver' => 'mysql',
  'database' => 'memo',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);
$databases['mig_memo_data']['default'] = array(
  'driver' => 'mysql',
  'database' => 'memo_data',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);

// In case you want to convert vpx_ftp.
$databases['mig_vpx_ftp']['default'] = array(
  'driver' => 'mysql',
  'database' => 'vpx_ftp',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);
 */

define('MIG_MEMO', variable_get('mediamosa_db_mig_memo', 'mig_memo'));
define('MIG_MEMO_DATA', variable_get('mediamosa_db_mig_memo_data', 'mig_memo_data'));
define('MIG_VPX_FTP', variable_get('mediamosa_db_mig_vpx_ftp', 'mig_vpx_ftp'));

/**
 * Get the database name of the mediamosa drupal database.
 *
 * @return string
 */
function _mediamosa_migration_get_memo_db() {
  global $databases;
  return empty($databases[MIG_MEMO]['default']['database']) ? FALSE : $databases[MIG_MEMO]['default']['database'];
}

/**
 * Get the database name of the mediamosa data database.
 *
 * @return string
 */
function _mediamosa_migration_get_memo_data_db() {
  global $databases;
  return empty($databases[MIG_MEMO_DATA]['default']['database']) ? FALSE : $databases[MIG_MEMO_DATA]['default']['database'];
}

/**
 * Get the database name of the vpx_ftp data database.
 *
 * @return string
 */
function _mediamosa_migration_get_vpx_data_db() {
  global $databases;
  return empty($databases[MIG_VPX_FTP]['default']['database']) ? FALSE : $databases[MIG_VPX_FTP]['default']['database'];
}

/**
 * Test if all is ok to migrate
 */
function _mediamosa_requirements_test() {

  // These tables are required on the destination database.
  $table_names_dest_exists = array(
    'mediamosa_error',
    'mediamosa_asset',
    'mediamosa_asset_mediafile',
    'mediamosa_asset_mediafile_metadata',
    'mediamosa_asset_metadata',
    'mediamosa_asset_metadata_property',
    'mediamosa_asset_metadata_property_group',
    'mediamosa_acl_app_master_slave',
    'mediamosa_acl_group',
    'mediamosa_acl_name',
    'mediamosa_acl_object',
    'mediamosa_app',
    'mediamosa_collection',
    'mediamosa_server_streaming_container',
    'mediamosa_server_tool',
    'mediamosa_ticket',
    'mediamosa_ftp_batch',
    'mediamosa_ftp_batch_asset'
  );
  // These tables (dest) must be empty.
  $table_names_dest_empty = array(
    'mediamosa_asset',
    'mediamosa_asset_mediafile',
    'mediamosa_acl_app_master_slave',
    'mediamosa_acl_group',
    'mediamosa_acl_name',
    'mediamosa_acl_object',
    'mediamosa_collection',
    'mediamosa_ticket',
    'mediamosa_ftp_batch',
    'mediamosa_ftp_batch_asset',
  );
  // Must exists source drupal db.
  $table_names_source_exists = array(
    'error',
    'download_server',
    'still_server',
    'streaming_server',
    'streaming_server_container',
    'transcoding_server',
    'transcoding_server_tool',
    'upload_server',
    'transcode_profile',
    'transcode_mapping',
  );
  // Must exists source data db.
  $table_names_source_exists_data = array(
    'asset',
    'mediafile',
    'v_asset_property',
    'asset_property',
    'assetprop_definition',
    'assetprop_group',
    'aut_name',
    'aut_group',
    'aut_object',
    'aut_app_master_slave',
    'collection',
    'ticket',
    'quota_user',
    'quota_group',
    'ftp_batch',
    'ftp_batch_asset',
    'job',
    'still_job',
    'transcode_job',
    'upload_job',
  );

  // Possible reasons.
  $reasons = array();

  $old_db = 'default';
  try {
    _mediamosa_migration_tables_exists($table_names_dest_exists);
  }
  catch (Exception $e) {
    $reasons[] = $e->getMessage();
  }

  try {
  // Select the old mediamosa database.
    $old_db = db_set_active(MIG_MEMO_DATA);

    // Data source.
    _mediamosa_migration_tables_exists($table_names_source_exists_data, MIG_MEMO_DATA);
  }
  catch (Exception $e) {
    $reasons[] = $e->getMessage();
  }

  // Back to other.
  db_set_active($old_db);

  try {
    // Select the old mediamosa database.
    $old_db = db_set_active(MIG_MEMO);

    // Test if these tables exists.
    _mediamosa_migration_tables_exists($table_names_source_exists, MIG_MEMO);
  }
  catch (Exception $e) {
    $reasons[] = $e->getMessage();
  }

  // Back to other.
  db_set_active($old_db);

  try {
    // Test if the required destination tables ARE empty.
    _mediamosa_migration_empty_tables($table_names_dest_empty);
  }
  catch (Exception $e) {
    $reasons[] = $e->getMessage();
  }

  try {
    // Make sure the medafile_ids in mediafile 1.x table are unique.
    _mediamosa_migration_mediafileid_is_unique();
  }
  catch (Exception $e) {
    $reasons[] = $e->getMessage();
  }

  try {
    // Make sure these class can be found.
    if (!class_exists('mediamosa_asset_metadata_db')) {
      throw new Exception('Could not find class mediamosa_asset_metadata_db, make sure MediaMosa core is installed.');
    }
  }
  catch (Exception $e) {
    $reasons[] = $e->getMessage();
  }

  return $reasons;
}

/**
 * Check if a table exists in current database.
 */
function _mediamosa_db_table_exists($table_name, $key = 'default') {
  $info = Database::getConnectionInfo($key);

  $schema = $info['default']['database'];

  $condition = new DatabaseCondition('AND');
  $condition->condition('table_catalog', NULL, 'IS');
  $condition->condition('table_schema', $schema);
  $condition->condition('table_name', Database::getConnection('default', $key)->prefixTables('{' . $table_name . '}'));
  $condition->compile(Database::getConnection($key), Database::getConnection('default', $key)->schema());

  return db_query("SELECT table_name FROM information_schema.tables WHERE " . (string)$condition, $condition->arguments())->fetchField();
}

/**
 * Loop through table names and check if they exists.
 *
 * @param array $a_table_names
 */
function _mediamosa_migration_tables_exists($a_table_names, $key = 'default') {
  // Ensure translations don't break at install time.
  $t = get_t();

  foreach ($a_table_names as $table_name) {
    if (!_mediamosa_db_table_exists($table_name, $key)) {
      throw new Exception($t("The table '@table_name' does not exists.", array('@table_name' => $table_name)));
    }
  }
}

/**
 * Loop through table names and check if they exists.
 *
 * @param array $a_table_names
 */
function _mediamosa_migration_empty_tables($a_table_names, $key = 'default') {
  $old_db = db_set_active($key);

  // Ensure translations don't break at install time.
  $t = get_t();

  try {
    foreach ($a_table_names as $table_name) {
      $has_rows = (bool) db_query_range('SELECT 1 FROM ' . $table_name, 0, 1)->fetchField();
      if ($has_rows) {
        throw new Exception($t("The table '@table_name' must be empty.", array('@table_name' => $table_name)));
      }
    }
  }
  catch (Exception $e) {
    db_set_active($old_db);
    throw $e;
  }

  db_set_active($old_db);
}

/**
 * Check if the medafile 1.x table has unique mediafile ids.
 * This is because mediafile table has invalid primary key, which will be fixed in 2.x.
 */
function _mediamosa_migration_mediafileid_is_unique() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  $result = db_query(
    strtr(
      "SELECT COUNT(*) AS total, mediafile_id FROM #db_name.mediafile GROUP BY mediafile_id HAVING total > 1",
      array(
        '#db_name' => $db_name,
      )
    )
  )->fetchField();

  if ($result > 0) {
    // Ensure translations don't break at install time.
    $t = get_t();
    throw new Exception($t("The table 'mediafile' contains non-unique ids."));
  }
}


/**
 * Migrate the error table.
 */
function _mediamosa_migration_error() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_db();

  $current_ids = array_keys(db_query("SELECT code FROM {mediamosa_error}")->fetchAllAssoc('code'));

  // Copy all to new table.
  db_query(strtr("INSERT INTO {mediamosa_error} (name, code, message) (SELECT name, code, message FROM #db_name.error#where)",
    array(
      '#db_name' => $db_name,
      '#where' => count($current_ids) ? ' WHERE code NOT IN(' . implode(',', $current_ids) . ') ' : ' ',
    )
  ));
}

/**
 * Migrate the client_applications table.
 */
function _mediamosa_migration_client_applications() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_db();

  // Get the current apps.
  $migrate_apps = db_query(strtr("SELECT * FROM #db_name.client_applications", array('#db_name' => $db_name)))->fetchAllAssoc('id');

  // Now add back the apps as nodes.
  foreach ($migrate_apps as $app_id => $app) {
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_APP, $app->name);

    // not needed.
    unset($app->caid);
    unset($app->id);
    $app->app_id = $app_id;

    $app = (array)$app;
    foreach ($app as $key => $value) {
      if (in_array($key, array('active', 'allow_masterslave'))) {
        $value = $value ? 'TRUE' : 'FALSE';
      }

      if ($key == 'name') {
        $key = 'app_name';// rename.
      }

      $node->{$key} = $value;
    }

    // Save node.
    node_save($node);
  }
}

/**
 * Migrate the asset table.
 */
function _mediamosa_migration_asset() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // The mediamosa_asset table should be empty (its checked as requirement).
  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * testtag
   * locked (unused)
   * has_streamable_mediafiles
   * Columns renamed;
   * mediafile_duration = media_duration
   * mediafile_container_type = media_container_type
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset}
      (
        asset_id,
        parent_id,
        app_id,
        provider_id,
        owner_id,
        group_id,
        videotimestamp,
        videotimestampmodified,
        play_restriction_start,
        play_restriction_end,
        reference_id,
        isprivate,
        mediafile_duration,
        mediafile_container_type,
        created,
        changed,
        is_unappropriate,
        is_external,
        is_empty_asset,
        viewed,
        played,
        is_protected
      ) (SELECT
        asset_id,
        parent_id,
        app_id,
        provider_id,
        owner_id,
        group_id,
        DATE_SUB(videotimestamp, INTERVAL 2 HOUR),
        DATE_SUB(videotimestampmodified, INTERVAL 2 HOUR),
        play_restriction_start,
        play_restriction_end,
        reference_id,
        isprivate,
        mediafile_duration,
        mediafile_container_type,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR),
        is_unappropiate,
        is_external,
        is_empty_asset,
        viewed,
        played,
        is_protected
      FROM #db_name.asset ORDER BY parent_id, asset_id ASC)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Now copy all changed dates to overwrite empty created dates.
  db_query("UPDATE {mediamosa_asset} SET created = changed WHERE created = '0000-00-00 00:00:00' AND changed <> '0000-00-00 00:00:00'");

  // Now make sure created is not empty.
  db_query("UPDATE {mediamosa_asset} SET created = UTC_TIMESTAMP WHERE created = '0000-00-00 00:00:00'");

  // Now make sure changed is not empty.
  db_query("UPDATE {mediamosa_asset} SET changed = UTC_TIMESTAMP WHERE changed = '0000-00-00 00:00:00'");

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));

}

/**
 * Migrate the mediafile table.
 */
function _mediamosa_migration_mediafile() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Remove foreign key on asset_mediafile_metadata table.
  db_query('ALTER TABLE {mediamosa_asset_mediafile_metadata} DROP FOREIGN KEY fk_mediamosa_asset_mediafile_metadata_mediafile_id');

  // Remove the primary key so insert will go fine.
  db_query('ALTER TABLE {mediamosa_asset_mediafile} DROP PRIMARY KEY');

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // The mediamosa_asset_mediafile table should be empty (its checked as requirement).

  // Select the old mediamosa database.
  $old_db = db_set_active(MIG_MEMO_DATA);

  try {
    // Some installations are missing is_streamable(!).
    $is_streamable = db_field_exists('mediafile', 'is_streamable');
  }
  catch (Exception $e) {
    db_set_active($old_db);
    throw $e;
  }

  // Back to other.
  db_set_active($old_db);

  // Insert fields.
  $insert_fields = array(
    'mediafile_id',
    'asset_id',
    'mediafile_id_source',
    'tag',
    'app_id',
    'owner_id',
    'group_id',
    'is_original_file',
    'is_downloadable',
    'is_still',
    'filename',
    'uri',
    'sannas_mount_point',
    'transcode_profile_id',
    'tool',
    'command',
    'file_extension',
    'is_protected',
    'created',
    'changed',
    'transcode_inherits_acl',
  );

  // Select fields.
  $select_fields = array(
    'mediafile_id',
    'asset_id',
    'mediafile_source',
    'tag',
    'app_id',
    'owner_id',
    'group_id',
    'is_original_file',
    'is_downloadable',
    'is_still',
    'filename',
    'uri',
    'sannas_mount_point',
    'transcode_profile_id',
    'tool',
    'command',
    'file_extension',
    'is_protected',
    'DATE_SUB(created, INTERVAL 2 HOUR)',
    'DATE_SUB(changed, INTERVAL 2 HOUR)',
    'transcode_inherits_acl',
  );

  // If field is present, then migrate.
  if ($is_streamable) {
    $insert_fields[] = 'is_streamable';
    $select_fields[] = 'is_streamable';
  }

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * testtag
   * has_streamable_mediafiles
   * Columns renamed;
   * mediafile_id = mediafile_id
   * mediafile_source = mediafile_id_source
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_mediafile}
      (
        !insert_fields
      ) (SELECT
        !select_fields
      FROM #db_name.mediafile)",
    array(
      '#db_name' => $db_name,
      '!insert_fields' => implode(',', $insert_fields),
      '!select_fields' => implode(',', $select_fields),
    ))
  );

  // Now copy all changed dates to overwrite empty created dates.
  db_query("UPDATE {mediamosa_asset_mediafile} SET created = changed WHERE created = '0000-00-00 00:00:00' AND changed <> '0000-00-00 00:00:00'");

  // Now make sure created is not empty.
  db_query("UPDATE {mediamosa_asset_mediafile} SET created = UTC_TIMESTAMP WHERE created = '0000-00-00 00:00:00'");

  // Now make sure changed is not empty.
  db_query("UPDATE {mediamosa_asset_mediafile} SET changed = UTC_TIMESTAMP WHERE changed = '0000-00-00 00:00:00'");

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));

  // Add back the primary key.
  db_query('ALTER TABLE {mediamosa_asset_mediafile} ADD PRIMARY KEY(mediafile_id)');

  // Add the foreign key on asset_mediafile_metadata table.
  db_query('ALTER TABLE {mediamosa_asset_mediafile_metadata} ADD CONSTRAINT fk_mediamosa_asset_mediafile_metadata_mediafile_id FOREIGN KEY (mediafile_id) REFERENCES {mediamosa_asset_mediafile} (mediafile_id)');
}

/**
 * Migrate the mediafile metadata table.
 */
function _mediamosa_migration_mediafile_metadata() {

  // Set time limit higher here.
  set_time_limit(240);

  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Existing fields.
  $fields = array();
  $columns = mediamosa_db::db_get_columns_with_type('mediafile_metadata', $db_name);
  foreach ($columns as $row) {
    // Collect only real properties.
    if (
      $row->column_name != 'metadata_id' &&
      $row->column_name != 'mediafile_id' &&
      $row->column_name != 'created' &&
      $row->column_name != 'changed'
    ) {
      // Type is eg. "varchar(32)" or "int(10) unsigned" or "enum('FALSE','TRUE')"
      $type = drupal_strtolower($row->data_type);

      // Type formatting.
      if (strpos($type, 'int') !== FALSE) {
        $type = 'INT';
      }
      elseif (strpos($type, 'char') !== FALSE || strpos($type, 'binary') !== FALSE || $type == 'blob' || $type == 'text' || $type == 'enum') {
        $type = 'CHAR';
      }
      else {
        $type = 'DATETIME';
      }

      // Insert.
      $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($row->column_name, $type);
      $fields[$row->column_name] = array(
        'prop_id' => $prop_id,
        'type' => $type,
      );
    }
  }

  // Migrating data.
  foreach ($fields as $field => $property) {
    switch ($property['type']) {
      case 'CHAR':
        db_query(strtr(
          "INSERT INTO {mediamosa_asset_mediafile_metadata}
            (
              mediafile_id,
              prop_id,
              val_char
            ) (SELECT
              mediafile_id,
              :prop_id,
              #val_char
            FROM #db_name.mediafile_metadata WHERE #val_char IS NOT NULL && #val_char != '' GROUP BY mediafile_id ORDER BY created ASC)",
          array(
            '#db_name' => $db_name,
            ':prop_id' => $property['prop_id'],
            '#val_char' => $field,
          ))
        );
        break;

      case 'DATETIME':
        db_query(strtr(
          "INSERT INTO {mediamosa_asset_mediafile_metadata}
            (
              mediafile_id,
              prop_id,
              val_date
            ) (SELECT
              mediafile_id,
              :prop_id,
              #val_date
            FROM #db_name.mediafile_metadata WHERE #val_date IS NOT NULL && #val_date != '0000-00-00 00:00:00' GROUP BY mediafile_id ORDER BY created ASC)",
          array(
            '#db_name' => $db_name,
            ':prop_id' => $property['prop_id'],
            '#val_date' => $field,
          ))
        );
        break;

      case 'INT':
        db_query(strtr(
          "INSERT INTO {mediamosa_asset_mediafile_metadata}
            (
              mediafile_id,
              prop_id,
              val_int
            ) (SELECT
              mediafile_id,
              :prop_id,
              #val_int
            FROM #db_name.mediafile_metadata WHERE #val_int IS NOT NULL GROUP BY mediafile_id ORDER BY created ASC)",
          array(
            '#db_name' => $db_name,
            ':prop_id' => $property['prop_id'],
            '#val_int' => $field,
          ))
        );
        break;
    }
  }
}

/**
 * Migrate the asset metadata property group table.
 */
function _mediamosa_migration_asset_metadata_property_group() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // Must be empty.
  db_truncate('mediamosa_asset_metadata_property')->execute();
  db_truncate('mediamosa_asset_metadata_property_group')->execute();

  // The mediamosa_asset_mediafile table should be empty (its checked as requirement).
  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * name = propgroup_name
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_metadata_property_group}
      (
        propgroup_id,
        propgroup_name,
        created,
        changed
      ) (SELECT
        propgroup_id,
        name,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
        FROM #db_name.assetprop_group)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Now copy all changed dates to overwrite empty created dates.
  db_query("UPDATE {mediamosa_asset_metadata_property_group} SET created = changed WHERE created = '0000-00-00 00:00:00' AND changed <> '0000-00-00 00:00:00'");

  // Now make sure created is not empty.
  db_query("UPDATE {mediamosa_asset_metadata_property_group} SET created = UTC_TIMESTAMP WHERE created = '0000-00-00 00:00:00'");

  // Now make sure changed is not empty.
  db_query("UPDATE {mediamosa_asset_metadata_property_group} SET changed = UTC_TIMESTAMP WHERE changed = '0000-00-00 00:00:00'");

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate the asset metadata property table.
 */
function _mediamosa_migration_asset_metadata_property() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // Must be empty.
  db_truncate('mediamosa_asset_metadata_property')->execute();

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * name = prop_name
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_metadata_property}
      (
        prop_id,
        propgroup_id,
        app_id,
        prop_name,
        type,
        min_occurences,
        max_occurences,
        created,
        changed
      ) (SELECT
        prop_id,
        propgroup_id,
        app_id,
        name,
        type,
        min_occurences,
        max_occurences,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
        FROM #db_name.assetprop_definition)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Now copy all changed dates to overwrite empty created dates.
  db_query("UPDATE {mediamosa_asset_metadata_property} SET created = changed WHERE created = '0000-00-00 00:00:00' AND changed <> '0000-00-00 00:00:00'");

  // Now make sure created is not empty.
  db_query("UPDATE {mediamosa_asset_metadata_property} SET created = UTC_TIMESTAMP WHERE created = '0000-00-00 00:00:00'");

  // Now make sure changed is not empty.
  db_query("UPDATE {mediamosa_asset_metadata_property} SET changed = UTC_TIMESTAMP WHERE changed = '0000-00-00 00:00:00'");

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate the asset metadata table.
 */
function _mediamosa_migration_asset_metadata() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // The mediamosa_asset_mediafile table should be empty (its checked as requirement).
  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * id -> metadata_id
   * -
   * Columns new;
   * val_char_lft
   * val_char_rght
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_metadata}
      (
        metadata_id,
        asset_id,
        prop_id,
        val_char,
        val_char_lft,
        val_char_rght,
        val_datetime,
        val_int,
        created,
        changed
      ) (SELECT
        apd.id,
        apd.asset_id,
        apd.prop_id,
        vap.val_char,
        left(vap.val_char, #lenlft),
        reverse(left(vap.val_char, #lenrght)),
        DATE_SUB(apd.val_datetime, INTERVAL 2 HOUR),
        apd.val_int,
        DATE_SUB(apd.created, INTERVAL 2 HOUR),
        DATE_SUB(apd.changed, INTERVAL 2 HOUR)
        FROM #db_name.asset_property AS apd JOIN #db_name.v_asset_property AS vap USING(id))",
    array(
      '#db_name' => $db_name,
      '#lenlft' => mediamosa_asset_metadata_db::VAL_CHAR_LFT_LENGTH,
      '#lenrght' => mediamosa_asset_metadata_db::VAL_CHAR_RGHT_LENGTH,
    ))
  );

  // Now copy all changed dates to overwrite empty created dates.
  db_query("UPDATE {mediamosa_asset_metadata_property} SET created = changed WHERE created = '0000-00-00 00:00:00' AND changed <> '0000-00-00 00:00:00'");

  // Now make sure created is not empty.
  db_query("UPDATE {mediamosa_asset_metadata_property} SET created = UTC_TIMESTAMP WHERE created = '0000-00-00 00:00:00'");

  // Now make sure changed is not empty.
  db_query("UPDATE {mediamosa_asset_metadata_property} SET changed = UTC_TIMESTAMP WHERE changed = '0000-00-00 00:00:00'");

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate the asset metadata table.
 */
function _mediamosa_migration_aut() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   * Columns new;
   * -
   */
  db_query(strtr(
    'INSERT INTO {mediamosa_acl_name}
      (
        acl_name_id,
        app_id,
        acl_group_id,
        acl_name,
        acl_prefix,
        acl_type
      ) (SELECT
        aut_name_id,
        app_id,
        aut_group_id,
        aut_name,
        aut_prefix,
        aut_type
        FROM #db_name.aut_name)',
    array(
      '#db_name' => $db_name,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   * Columns new;
   * -
   */
  db_query(strtr(
    'INSERT INTO {mediamosa_acl_group}
      (
        acl_group_id,
        app_id,
        acl_group_name,
        acl_group_type
      ) (SELECT
        aut_group_id,
        app_id,
        aut_group_name,
        aut_group_type
        FROM #db_name.aut_group)',
    array(
      '#db_name' => $db_name,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   * Columns new;
   * -
   */
  db_query(strtr(
    'INSERT INTO {mediamosa_acl_object}
      (
        acl_object_id,
        acl_object_type,
        acl_id,
        acl_type
      ) (SELECT
        aut_object_id,
        aut_object_type,
        aut_id,
        aut_type
        FROM #db_name.aut_object} GROUP BY aut_object_type, aut_object_id, aut_type, aut_id)',
    array(
      '#db_name' => $db_name,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * acl_id_slave -> acl_slave_id
   * acl_id_master -> acl_master_id
   * Columns new;
   * -
   */
  db_query(strtr(
    'INSERT INTO {mediamosa_acl_app_master_slave}
      (
        acl_object_id,
        acl_object_type,
        app_id_slave,
        app_id_master
      ) (SELECT
        aut_object_id,
        aut_object_type,
        app_slave_id,
        app_master_id
        FROM #db_name.aut_app_master_slave)',
    array(
      '#db_name' => $db_name,
    ))
  );
}

/**
 * Migrate san nas settings.
 */
function _mediamosa_migration_sannas() {

  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_db();

  $a_san_nas = db_query(
    strtr(
      'SELECT current_mount_point, current_mount_point_windows FROM #db_name.san_nas_settings LIMIT 1',
      array(
      '#db_name' => $db_name
      )
    )
  )->fetchAssoc();

  if ($a_san_nas) {
    mediamosa_configuration_storage::linux_set($a_san_nas['current_mount_point']);
    mediamosa_configuration_storage::windows_set($a_san_nas['current_mount_point_windows']);
  }
}

/**
 * Migrate collection table..
 */
function _mediamosa_migration_collection() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * testtag
   * has_streamable_mediafiles
   * Columns renamed;
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_collection}
      (
        coll_id,
        title,
        description,
        app_id,
        owner_id,
        group_id,
        isprivate,
        private,
        public,
        category,
        created,
        changed,
        public_assign,
        is_unappropriate
      ) (SELECT
        coll_id,
        title,
        description,
        app_id,
        owner_id,
        group_id,
        isprivate,
        private,
        public,
        category,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR),
        public_assign,
        is_unappropriate
      FROM #db_name.collection)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * has_streamable_mediafiles
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_collection}
      (
        asset_id,
        coll_id,
        created,
        changed,
        is_empty_asset
      ) (SELECT
        ac.asset_id,
        ac.coll_id,
        DATE_SUB(ac.created, INTERVAL 2 HOUR),
        DATE_SUB(ac.changed, INTERVAL 2 HOUR),
        a.is_empty_asset
      FROM #db_name.asset_collection AS ac JOIN #db_name.asset AS a ON (a.asset_id = ac.asset_id))",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate asset_delete table..
 */
function _mediamosa_migration_asset_delete() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_delete}
      (
        asset_id,
        app_id,
        videotimestampmodified,
        coll_id
      ) (SELECT
        asset_id,
        app_id,
        DATE_SUB(videotimestampmodified, INTERVAL 2 HOUR),
        coll_id
      FROM #db_name.asset_delete)",
    array(
      '#db_name' => $db_name
    ))
  );
}

/**
 * Migrate user/group table..
 */
function _mediamosa_migration_user() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * group_id
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_user}
      (
        app_id,
        name,
        quotum,
        created,
        changed
      ) (SELECT
        app_id,
        name,
        quotum,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
      FROM #db_name.quota_user)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_user_group}
      (
        app_id,
        group_id,
        quotum,
        created,
        changed
      ) (SELECT
        app_id,
        group_id,
        quotum,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
      FROM #db_name.quota_group)",
    array(
      '#db_name' => $db_name
    ))
  );

  $users_in_group = db_query(
    strtr("SELECT g.group_key, mu.user_key FROM #db_name.quota_user AS u JOIN {mediamosa_user} AS mu ON mu.name = u.name JOIN {mediamosa_user_group} AS g ON u.group_id = g.group_id AND u.app_id = g.app_id",
      array(
        '#db_name' => $db_name
      )
    )
  )->fetchAll();

  foreach ($users_in_group as $item) {
    db_query(
      strtr("INSERT INTO {mediamosa_user_in_group} (#field_user_key, #field_group_key) VALUES (#user_key, #group_key)",
        array(
          '#table_name' => 'mediamosa_user_in_group',
          '#field_user_key' => 'user_key',
          '#field_group_key' => 'group_key',
          '#user_key' => $item->user_key,
          '#group_key' => $item->group_key,
        )
      )
    );
  }

  /**
   * user_favorites -> mediamosa_user_favorite
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_user_favorite}
      (
        name,
        app_id,
        fav_type,
        fav_id
      ) (SELECT
        name,
        app_id,
        fav_type,
        fav_id
      FROM #db_name.user_favorites)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate ticket table..
 */
function _mediamosa_migration_ticket() {

  return TRUE; // why would we ever want to migrate this?

  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * user_id -> owner_id
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_ticket}
      (
        ticket_id,
        ticket_type,
        issued,
        app_id,
        owner_id,
        group_id,
        mediafile_id,
        created,
        changed
      ) (SELECT
        ticket_id,
        ticket_type,
        DATE_SUB(issued, INTERVAL 2 HOUR),
        app_id,
        user_id,
        group_id,
        mediafile_id,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
      FROM #db_name.ticket)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate ftp_batch table..
 */
function _mediamosa_migration_ftp_batch() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_ftp_batch}
      (
        batch_id,
        app_id,
        owner_id,
        group_id,
        vuf,
        started,
        finished,
        email_address,
        email_contents
      ) (SELECT
        batch_id,
        app_id,
        owner_id,
        group_id,
        vuf,
        DATE_SUB(started, INTERVAL 2 HOUR),
        DATE_SUB(finished, INTERVAL 2 HOUR),
        email_address,
        email_contents
      FROM #db_name.ftp_batch)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * id (autoinc not needed in a link table)
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_ftp_batch_asset}
      (
        asset_id,
        batch_id
      ) (SELECT
        asset_id,
        batch_id
      FROM #db_name.ftp_batch_asset GROUP BY batch_id, asset_id)",
    array(
      '#db_name' => $db_name
    ))
  );
}

/**
 * Migrate webservice table..
 */
function _mediamosa_migration_webservice() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_db();

  // Clean the tables.
  $a_to_truncate = array(
    'mediamosa_webservice',
    'mediamosa_webservice_app',
  );

  foreach ($a_to_truncate as $table_name) {
    db_truncate($table_name)->execute();
  }

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_webservice}
      (
        webservice_id,
        version,
        handle,
        description,
        status
      ) (SELECT
        webservice_management_id,
        version,
        handle,
        description,
        status
      FROM #db_name.webservice_management)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   *
   * Will only copy which have a handle in mediamosa_webservice and valid app_id linked to mediamosa_app.
   *
   * Columns dropped;
   * -
   * Columns renamed;
   * wmcaid - webservice_app_id
   * webservice_management_handle - webservice_handle
   * caid - app_id
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_webservice_app}
      (
        webservice_app_id,
        webservice_handle,
        app_id,
        status
      ) (SELECT
        wa.wmcaid,
        wa.webservice_management_handle,
        wa.caid,
        wa.status
        FROM #db_name.webservice_management_capp AS wa JOIN {mediamosa_webservice} AS w ON w.handle = wa.webservice_management_handle JOIN mediamosa_app AS app ON app.app_id = wa.caid)",
    array(
      '#db_name' => $db_name
    ))
  );
}

/**
 * Migrate asset supplement table..
 */
function _mediamosa_migration_asset_supplement() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_asset_supplement}
      (
        supplement_id,
        asset_id,
        supplement,
        tag,
        oai_prefix,
        oai_namespaceuri,
        oai_schemalocation,
        created,
        changed
      ) (SELECT
        supplement_id,
        asset_id,
        supplement,
        tag,
        oai_prefix,
        oai_namespaceuri,
        oai_schemalocation,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
      FROM #db_name.asset_supplement)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate job tables.
 */
function _mediamosa_migration_job() {

  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * testtag
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_job}
      (
        job_id,
        asset_id,
        mediafile_id,
        owner,
        app_id,
        status,
        progress,
        priority,
        job_type,
        started,
        finished,
        error_description,
        created,
        changed,
        create_still,
        still_parameters
      ) (SELECT
        job_id,
        asset_id,
        mediafile_id,
        owner,
        app_id,
        status,
        progress,
        priority,
        job_type,
        DATE_SUB(started, INTERVAL 2 HOUR),
        DATE_SUB(finished, INTERVAL 2 HOUR),
        error_description,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR),
        create_still,
        still_parameters
      FROM #db_name.job WHERE status IN('FINISHED','FAILED') ORDER BY job_id ASC)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_job_still}
      (
        still_job_id,
        job_id,
        size,
        frametime,
        h_padding,
        v_padding,
        blackstill_check,
        still_parameters,
        created,
        changed
      ) (SELECT
        sj.still_job_id,
        sj.job_id,
        sj.size,
        sj.frametime,
        sj.h_padding,
        sj.v_padding,
        sj.blackstill_check,
        sj.still_parameters,
        DATE_SUB(sj.created, INTERVAL 2 HOUR),
        DATE_SUB(sj.changed, INTERVAL 2 HOUR)
    FROM #db_name.still_job AS sj JOIN {mediamosa_job} AS j ON (sj.job_id = j.job_id))",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_job_transcode}
      (
        transcode_job_id ,
        job_id,
        transcode_profile_id,
        tool,
        command,
        completed_transcoding_url,
        file_extension,
        created,
        changed
      ) (SELECT
        tj.transcode_job_id ,
        tj.job_id,
        tj.transcode_profile_id,
        tj.tool,
        tj.command,
        tj.completed_transcoding_url,
        tj.file_extension,
        DATE_SUB(tj.created, INTERVAL 2 HOUR),
        DATE_SUB(tj.changed, INTERVAL 2 HOUR)
    FROM #db_name.transcode_job AS tj JOIN {mediamosa_job} AS j ON (tj.job_id = j.job_id))",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_job_upload}
      (
        upload_job_id,
        job_id,
        file_size,
        uploaded_file_size,
        retranscode,
        create_still,
        created,
        changed
      ) (SELECT
        uj.upload_job_id,
        uj.job_id,
        uj.file_size,
        uj.uploaded_file_size,
        uj.retranscode,
        uj.create_still,
        DATE_SUB(uj.created, INTERVAL 2 HOUR),
        DATE_SUB(uj.changed, INTERVAL 2 HOUR)
    FROM #db_name.upload_job AS uj JOIN {mediamosa_job} AS j ON (uj.job_id = j.job_id))",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}


/**
 * Migrate jobserver tables.
 */
function _mediamosa_migration_jobserver_all() {

  global $databases;
  if (!isset($databases['jobserver']) && !isset($databases['jobserver']['default']) && !isset($databases['jobserver']['default']['mediamosa_jobserver_dbs'])) {
    return;
  }

  foreach ($databases['jobserver']['default']['mediamosa_jobserver_dbs'] as $from_dbname => $to_dbname) {
    if (is_numeric($from_dbname)) {
      assert(0); // missing key.
      continue;
    }

    //_mediamosa_migration_jobserver($from_dbname, $to_dbname);
  }
}

function _mediamosa_migration_jobserver($from_dbname, $to_dbname) {

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * testtag
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO from_dbname.{mediamosa_job_server}
      (
        jobserver_job_id,
        job_id,
        status,
        progress,
        job_type,
        progress_id,
        mediafile_src,
        mediafile_dest,
        started,
        finished,
        error_description,
        created,
        changed
      ) (SELECT
        jobserver_job_id,
        job_id,
        status,
        progress,
        job_type,
        progress_id,
        mediafile_src,
        mediafile_dest,
        DATE_SUB(started, INTERVAL 2 HOUR),
        DATE_SUB(finished, INTERVAL 2 HOUR),
        error_description,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
    FROM #to_dbname.jobserver_job)",
    array(
      '#from_dbname' => $from_dbname,
      '#to_dbname' => $to_dbname,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * testtag
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO from_dbname.{mediamosa_job_server}
      (
        jobserver_job_id,
        job_id,
        status,
        progress,
        job_type,
        progress_id,
        mediafile_src,
        mediafile_dest,
        started,
        finished,
        error_description,
        created,
        changed
      ) (SELECT
        jobserver_job_id,
        job_id,
        status,
        progress,
        job_type,
        progress_id,
        mediafile_src,
        mediafile_dest,
        DATE_SUB(started, INTERVAL 2 HOUR),
        DATE_SUB(finished, INTERVAL 2 HOUR),
        error_description,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
    FROM #to_dbname.jobserver_job)",
    array(
      '#from_dbname' => $from_dbname,
      '#to_dbname' => $to_dbname,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO from_dbname.{mediamosa_job_server_analyse}
      (
        jobserver_job_id,
        analyse_result,
        created,
        changed
      ) (SELECT
        jobserver_job_id,
        analyse_result,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
    FROM #to_dbname.{observer_analyse_job)",
    array(
      '#from_dbname' => $from_dbname,
      '#to_dbname' => $to_dbname,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO from_dbname.{mediamosa_job_server_still}
      (
        jobserver_job_id,
        frametime,
        size,
        h_padding,
        v_padding,
        created,
        changed,
        blackstill_check,
        still_parameters
      ) (SELECT
        jobserver_job_id,
        frametime,
        size,
        h_padding,
        v_padding,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR),
        blackstill_check,
        still_parameters
    FROM #to_dbname.jobserver_still_job)",
    array(
      '#from_dbname' => $from_dbname,
      '#to_dbname' => $to_dbname,
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO from_dbname.{mediamosa_job_server_transcode}
      (
        jobserver_job_id,
        frametime,
        size,
        h_padding,
        v_padding,
        created,
        changed,
        blackstill_check,
        still_parameters
      ) (SELECT
        jobserver_job_id,
        frametime,
        size,
        h_padding,
        v_padding,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR),
        blackstill_check,
        still_parameters
    FROM #to_dbname.jobserver_transcode_job)",
    array(
      '#from_dbname' => $from_dbname,
      '#to_dbname' => $to_dbname,
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate transcode_* tables.
 */
function _mediamosa_migration_transcode() {

  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // Empty first.
  $a_all = db_select('mediamosa_transcode_profile', 'mtp')->fields('mtp')->execute()->fetchAll();
  foreach ($a_all as $transcode_profile) {
    node_delete($transcode_profile->nid);
  }

  // Get the transcode_profiles.
  $result = mediamosa_db::db_query(
    'SELECT * FROM #db_name.transcode_profile',
    array(
      '#db_name' => $db_name,
    )
  );

  foreach ($result as $row) {
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE, $row[mediamosa_transcode_profile_db::PROFILE]);

    $node->{mediamosa_transcode_profile_db::ID} = $row[mediamosa_transcode_profile_db::ID];
    $node->{mediamosa_transcode_profile_db::APP_ID} = $row[mediamosa_transcode_profile_db::APP_ID];
    $node->{mediamosa_transcode_profile_db::VERSION} = $row[mediamosa_transcode_profile_db::VERSION];
    $node->{mediamosa_transcode_profile_db::PROFILE} = $row[mediamosa_transcode_profile_db::PROFILE];
    $node->{mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE} = $row[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE];
    $node->{mediamosa_transcode_profile_db::TOOL} = $row[mediamosa_transcode_profile_db::TOOL];
    $node->{mediamosa_transcode_profile_db::FILE_EXTENSION} = $row[mediamosa_transcode_profile_db::FILE_EXTENSION];
    $node->{mediamosa_transcode_profile_db::COMMAND} = $row[mediamosa_transcode_profile_db::COMMAND];

    $node = node_save($node);
  }


  /**
   * Following code is off because the mapping is always old in the older databases.
   * TODO: allowed_value is serialized array now; needs conversation from ";" separated string.
   */
/*
  // Empty first.
  db_truncate('mediamosa_tool_params')->execute();
*/
  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
/*
  db_query(strtr(
    "INSERT INTO {mediamosa_tool_params}
      (
        transcode_mapping_id,
        tool,
        nice_parameter,
        tool_parameter,
        min_value,
        max_value,
        allowed_value,
        default_value,
        required,
        created,
        changed
      ) (SELECT
        transcode_mapping_id,
        tool,
        nice_parameter,
        tool_parameter,
        min_value,
        max_value,
        allowed_value,
        default_value,
        required,
        DATE_SUB(created, INTERVAL 2 HOUR),
        DATE_SUB(changed, INTERVAL 2 HOUR)
    FROM #db_name.transcode_mapping)",
    array(
      '#db_name' => $db_name
    ))
  );
*/
  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

/**
 * Migrate server table to nodes mediamosa_server.
 */
function _mediamosa_migration_create_server_nodes() {

  // Empty first.
  $a_all = db_select('mediamosa_server', 's')->fields('s')->execute()->fetchAll();
  foreach ($a_all as $mediamosa_server) {
    node_delete($mediamosa_server->nid);
  }

  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_db();

  // Empty first.
  $a_all = db_select('mediamosa_server', 'ms')->fields('ms')->execute()->fetchAll();
  foreach ($a_all as $o_server) {
    node_delete($o_server->nid);
  }

  $tables = array(
    mediamosa_server_db::SERVER_TYPE_DOWNLOAD => 'download_server',
    mediamosa_server_db::SERVER_TYPE_STILL => 'still_server',
    mediamosa_server_db::SERVER_TYPE_STREAMING => 'streaming_server',
    mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR => 'transcoding_server',
    mediamosa_server_db::SERVER_TYPE_UPLOAD => 'upload_server',
  );

  $a_streaming_server_containers = db_query(strtr('SELECT * FROM #db_name.streaming_server_container', array('#db_name' => $db_name)))->fetchAll();
  $a_transcoding_server_tool = db_query(strtr('SELECT * FROM #db_name.transcoding_server_tool', array('#db_name' => $db_name)))->fetchAll();

  foreach ($tables as $type => $table) {
    $result = db_query(
      strtr('SELECT * FROM #db_name.#table_name',
      array(
        '#table_name' => $table,
        '#db_name' => $db_name,
      )),
      array(),
      array(
        'fetch' => PDO::FETCH_ASSOC,
      )
    );

    foreach ($result as $row) {
      $title = (isset($row['servername']) ? $row['servername'] : (isset($row['description']) ? $row['description'] : NULL));
      $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER, $title);

      // Find server_id
      $server_id = 0;
      foreach ($row as $key => $value) {
        if (preg_match('/_server_id$/', $key)) {
          $server_id = $value;
          break;
        }
      }

      $status = 'OFF';
      if (isset($row['active'])) {
        $status = $row['active'] ? 'ON' : 'OFF';
      }
      elseif (isset($row['status'])) {
        $status = (!is_numeric($row['status']) ? $row['status'] : ($row['status'] == 1 ? 'ON' : 'OFF'));
      }

      $parsed_uri = $parsed_path = parse_url(isset($row['uri']) ? $row['uri'] : (isset($row['ip_address']) ? $row['ip_address'] : NULL));

      unset($parsed_uri['path']);
      unset($parsed_uri['query']);
      unset($parsed_uri['fragment']);
      $server_uri = mediamosa_http::glue_url($parsed_uri);

      unset($parsed_path['scheme']);
      unset($parsed_path['host']);
      unset($parsed_path['user']);
      unset($parsed_path['pass']);
      unset($parsed_path['port']);
      $server_path = '{base_uri}' . mediamosa_http::glue_url($parsed_path);

      $node->{mediamosa_server_db::SERVER_TYPE} = $type;
      $node->{mediamosa_server_db::VERSION} = $row['version'];
      $node->{mediamosa_server_db::SERVER_STATUS} = $status;
      $node->{mediamosa_server_db::SERVER_URI} = $server_uri;
      $node->{mediamosa_server_db::SERVER_PATH} = $server_path;
      $node->{mediamosa_server_db::DESCRIPTION} = (isset($row['description']) ? $row['description'] : NULL);
      $node->{mediamosa_server_db::CONTAINERS} = (isset($row['containers']) ? $row['containers'] : NULL);
      $node->{mediamosa_server_db::OBJECT_CODE} = (isset($row['objectcode']) ? $row['objectcode'] : NULL);
      $node->{mediamosa_server_db::SERVER_NAME} = $title;
      $node->{mediamosa_server_db::SLOTS} = (isset($row['slots']) ? $row['slots'] : NULL);
      $node->{mediamosa_server_db::TOOLS} = (isset($row['tools']) ? $row['tools'] : NULL);
      $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS} = (isset($row['uri_uploadprogress']) ? $row['uri_uploadprogress'] : NULL);

      node_save($node);
    }
  }
}

/**
 * Migrate childs asset mediafiles to the root.
 * Remove childs assets.
 */
function _mediamosa_migration_kill_child_assets() {

  // Lets increase to 10 minutes, although db time isn't counted.
  set_time_limit(600);

  // First fix the mediamosa_source_id to point to its original.

  // Get all orginals that are have URI on empty.
  $count = db_query("SELECT COUNT(mediafile_ID) FROM {mediamosa_asset_mediafile} WHERE (uri = '' OR uri IS NULL) AND is_original_file = 'TRUE' AND is_still = 'FALSE'")->fetchField();

  $per = 1000;
  for ($x = 0; $x < $count; $x += $per) {
    // Get per 1000 the original IDs.
    $a_mediafile_originals = db_query_range("SELECT mediafile_id, asset_id FROM {mediamosa_asset_mediafile} WHERE (uri = '' OR uri IS NULL) AND is_original_file = 'TRUE' AND is_still = 'FALSE' GROUP BY asset_id ORDER BY created ASC, mediafile_id ASC", $x, $per)->fetchAllKeyed();

    // Now loop through the originals and set the mediafile_source_id to the original.
    foreach ($a_mediafile_originals as $mediafile_id => $asset_id) {
      db_query("UPDATE {mediamosa_asset_mediafile} SET mediafile_id_source = :mediafile_id_source WHERE asset_id = :asset_id AND (URI = '' OR URI IS NULL) AND is_original_file = 'FALSE' AND mediafile_id_source IS NULL", array(':mediafile_id_source' => $mediafile_id, ':asset_id' => $asset_id));
    }

    // Clear mem.
    unset($a_mediafile_originals);
  }

  // Get all orginals that are have URI on empty.
  $count = db_query("SELECT COUNT(mediafile_ID) FROM {mediamosa_asset_mediafile} WHERE (uri = '' OR uri IS NULL) AND is_original_file = 'TRUE' AND is_still = 'FALSE'")->fetchField();

  $per = 1000;
  for ($x = 0; $x < $count; $x += $per) {
    // Get per 1000 the original IDs.
    $a_mediafile_originals = db_query_range("SELECT mediafile_id, asset_id FROM {mediamosa_asset_mediafile} WHERE (uri = '' OR uri IS NULL) AND is_original_file = 'TRUE' AND is_still = 'FALSE' GROUP BY asset_id ORDER BY created ASC, mediafile_id ASC", $x, $per)->fetchAllKeyed();

    // Now loop through the originals and set the mediafile_source_id to the original.
    foreach ($a_mediafile_originals as $mediafile_id => $asset_id) {
      db_query("UPDATE {mediamosa_asset_mediafile} SET mediafile_id_source = :mediafile_id_source WHERE asset_id = :asset_id AND (URI = '' OR URI IS NULL) AND is_original_file = 'FALSE' AND mediafile_id_source IS NULL", array(':mediafile_id_source' => $mediafile_id, ':asset_id' => $asset_id));
    }

    // Clear mem.
    unset($a_mediafile_originals);
  }
}

function _mediamosa_migration_statistics() {
  // Get the database name.
  $db_name = _mediamosa_migration_get_memo_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // Must be empty.
  db_truncate('mediamosa_statistics_diskspace_used')->execute();
  db_truncate('mediamosa_statistics_file_upload')->execute();
  db_truncate('mediamosa_statistics_rest_log')->execute();
  db_truncate('mediamosa_statistics_search_request')->execute();

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_statistics_diskspace_used}
      (
        id,
        app_id,
        type,
        keyword,
        container_type,
        diskspace,
        timestamp
      ) (SELECT
        id,
        app_id,
        type,
        keyword,
        container_type,
        diskspace,
        DATE_SUB(timestamp, INTERVAL 2 HOUR)
        FROM #db_name.statistics_diskspace_used)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_statistics_file_upload}
      (
        id,
        app_id,
        owner_id,
        group_id,
        file_size,
        timestamp
        ) (SELECT
        id,
        app_id,
        owner_id,
        group_id,
        file_size,
        DATE_SUB(timestamp, INTERVAL 2 HOUR)
        FROM #db_name.statistics_file_upload)",
    array(
      '#db_name' => $db_name
    ))
  );

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_statistics_rest_log}
      (
        id,
        request,
        process_time,
        query_count
      ) (SELECT
        id,
        request,
        process_time,
        query_count
        FROM #db_name.statistics_rest_log)",
    array(
      '#db_name' => $db_name
    ))
  );

    /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_statistics_search_request}
      (
        id,
        keyword,
        app_id,
        searched
      ) (SELECT
        id,
        keyword,
        app_id,
        searched
        FROM #db_name.statistics_search_request)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

function _mediamosa_migration_vpx_ftp() {

  try {
    // If it doesn't exists, we dont care.
    if (!_mediamosa_db_table_exists('ftpuser', MIG_VPX_FTP)) {
      return;
    }
  }
  catch (PDOException $e) {
    assert($e);
    return; // Ignore.
  }
  // Get the database name.
  $db_name = _mediamosa_migration_get_vpx_data_db();

  // Get the current mode, which might be empty;
  $old_mode = db_query('SELECT @@SESSION.sql_mode')->fetchField();

  // Turn off problems with empty dates.
  db_query("SET SESSION sql_mode = 'ALLOW_INVALID_DATES'");

  // Must be empty.
  db_truncate(mediamosa_ftp_user_db::TABLE_NAME)->execute();

  /**
   * Copy from 1.x to 2.x
   * Columns dropped;
   * -
   * Columns renamed;
   * -
   */
  db_query(strtr(
    "INSERT INTO {mediamosa_ftp_user}
      (
        id,
        app_id,
        userid,
        passwd,
        uid,
        gid,
        homedir,
        shell,
        active,
        count,
        accessed,
        modified
      ) (SELECT
        id,
        eua_id,
        userid,
        passwd,
        uid,
        gid,
        homedir,
        shell,
        active,
        count,
        DATE_SUB(accessed, INTERVAL 2 HOUR),
        DATE_SUB(modified, INTERVAL 2 HOUR)
        FROM #db_name.ftpuser)",
    array(
      '#db_name' => $db_name
    ))
  );

  // Back to old session mode.
  db_query(strtr("SET SESSION sql_mode = '#old_mode'", array('#old_mode' => $old_mode)));
}

function _mediamosa_migration_settings() {
  // Migrate this setting.
  $curlproxy_http = variable_get('vpx_curlproxy_http', '');
  if ($curlproxy_http != '') {
    variable_set('mediamosa_curlproxy_http', $curlproxy_http);
  }
}
