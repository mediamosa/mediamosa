<?php
/**
 * @file
 * Core - Features export and import hooks for MediaMosa server objects.
 */

/**
 * Implements hook_features_export_options().
 */
function mediamosa_server_features_export_options() {
  $options = array();

  $servers = mediamosa_server::get_enabled(array(), array());
  foreach ($servers as $server) {
    $options[$server[mediamosa_server_db::MACHINE_NAME]] = $server[mediamosa_server_db::SERVER_NAME];
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function mediamosa_server_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ($data as $machine_name) {
    $export['dependencies']['mediamosa'] = 'mediamosa';
    $export['features'][FEATURE_MEDIAMOSA_SERVER][$machine_name] = $machine_name;
  }
  return $pipe;
}

/**
 * Implements hook_features_export_render()
 */
function mediamosa_server_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $' . FEATURE_MEDIAMOSA_SERVER . 's = array();';
  $code[] = '';

  foreach ($data as $machine_name) {
    $row = db_select('mediamosa_server', 'ms')
      ->fields('ms')
      ->condition('machine_name', $machine_name)
      ->orderBy('machine_name')
      ->execute()
      ->fetchAssoc();
    if ($row) {
      unset($row['nid'], $row['vid']);
      $export = features_var_export($row, '  ');
      $code[] = "  // Exported mediamosa server: {$row['server_name']}.";
      $code[] = "  \$" . FEATURE_MEDIAMOSA_SERVER . "s['{$machine_name}'] = {$export};";
    }
  }

  $code[] = '';
  $code[] = '  return $' . FEATURE_MEDIAMOSA_SERVER . 's;';
  $code = implode("\n", $code);
  return array(FEATURE_MEDIAMOSA_SERVER . 's' => $code);
}

/**
 * Implements hook_features_revert().
 *
 * @param string $module_name
 *   The name of the feature module whose components should be reverted.
 */
function mediamosa_server_features_revert($module_name) {
  $mediamosa_servers = features_get_default(FEATURE_MEDIAMOSA_SERVER, $module_name);

  $fields = mediamosa_server_db::get_fields();
  foreach ($mediamosa_servers as $machine_name => $mediamosa_server) {
    $mediamosa_server_loaded = mediamosa_server::get_server_by_machinename($machine_name);

    if (empty($mediamosa_server_loaded)) {
      // Create new server.
      $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER, $mediamosa_server[mediamosa_server_db::SERVER_NAME]);
    }
    else {
      $node = node_load($mediamosa_server_loaded['nid']);
    }

    foreach ($mediamosa_server as $field => $value) {
      if (in_array($field, $fields)) {
        $node->{$field} = $value;
      }
    }

    // We copy the node title to server name in our node code.
    $node->title = $mediamosa_server[mediamosa_server_db::SERVER_NAME];

    node_save($node);
  }
}
