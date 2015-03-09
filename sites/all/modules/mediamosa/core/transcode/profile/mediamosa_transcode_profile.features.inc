<?php
/**
 * @file
 * Core - Features export and import hooks for MediaMosa server objects.
 */

/**
 * Implements hook_features_export_options().
 */
function mediamosa_transcode_profile_features_export_options() {
  $options = array();

  $transcode_profiles = mediamosa_transcode_profile::get_all();
  foreach ($transcode_profiles as $transcode_profile) {
    $options[$transcode_profile[mediamosa_transcode_profile_db::MACHINE_NAME]] = $transcode_profile[mediamosa_transcode_profile_db::PROFILE];
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function mediamosa_transcode_profile_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ($data as $machine_name) {
    $export['dependencies']['mediamosa'] = 'mediamosa';
    $export['features'][FEATURE_MEDIAMOSA_TRANSCODE_PROFILE][$machine_name] = $machine_name;
  }
  return $pipe;
}

/**
 * Implements hook_features_export_render()
 */
function mediamosa_transcode_profile_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $mediamosa_transcode_profiles = array();';
  $code[] = '';

  foreach ($data as $machine_name) {
    $row = db_select('mediamosa_transcode_profile', 'mtp')
      ->fields('mtp')
      ->condition('machine_name', $machine_name)
      ->orderBy('machine_name')
      ->execute()
      ->fetchAssoc();
    if ($row) {
      unset($row['nid'], $row['vid'], $row['transcode_profile_id'], $row['changed'], $row['created']);
      $export = features_var_export($row, '  ');
      $code[] = "  // Exported mediamosa transcode profile: {$row['profile']}.";
      $code[] = "  \$mediamosa_transcode_profiles['{$machine_name}'] = {$export};";
    }
  }

  $code[] = '';
  $code[] = '  return $mediamosa_transcode_profiles;';
  $code = implode("\n", $code);
  return array('mediamosa_transcode_profiles' => $code);
}

/**
 * Implements hook_features_revert().
 *
 * @param string $module_name
 *   The name of the feature module whose components should be reverted.
 */
function mediamosa_transcode_profile_features_revert($module_name) {
  $mediamosa_transcode_profiles = features_get_default(FEATURE_MEDIAMOSA_TRANSCODE_PROFILE, $module_name);

  $fields = mediamosa_transcode_profile_db::get_fields();
  foreach ($mediamosa_transcode_profiles as $machine_name => $mediamosa_transcode_profile) {
    $mediamosa_transcode_profile_loaded = mediamosa_transcode_profile::get_by_machine_name($machine_name);

    if (empty($mediamosa_transcode_profile_loaded)) {
      // Create new server.
      $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE, $mediamosa_transcode_profile[mediamosa_transcode_profile_db::PROFILE]);
    }
    else {
      $node = node_load($mediamosa_transcode_profile_loaded['nid']);
    }

    foreach ($mediamosa_transcode_profile as $field => $value) {
      if (in_array($field, $fields)) {
        $node->{$field} = $value;
      }
    }

    // We copy the node title to server name in our node code.
    $node->title = $mediamosa_transcode_profile[mediamosa_transcode_profile_db::PROFILE];

    node_save($node);
  }
}
