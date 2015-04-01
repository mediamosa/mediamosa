<?php
/**
 * @file
 * Core - Features export and import hooks for MediaMosa viewers.
 */

/**
 * Implements hook_features_export_options().
 */
function mediamosa_viewer_features_export_options() {
  $options = array();

  $viewers = mediamosa_viewer::search();
  foreach ($viewers as $viewer) {
    $options[$viewer[mediamosa_viewer_db::MACHINE_NAME]] = $viewer[mediamosa_viewer_db::NAME];
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function mediamosa_viewer_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ($data as $machine_name) {
    $export['dependencies']['mediamosa'] = 'mediamosa';
    $export['features'][FEATURE_MEDIAMOSA_VIEWER][$machine_name] = $machine_name;
  }
  return $pipe;
}

/**
 * Implements hook_features_export_render().
 */
function mediamosa_viewer_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $' . FEATURE_MEDIAMOSA_VIEWER . 's = array();';
  $code[] = '';
  foreach ($data as $machine_name) {
    $row = mediamosa_viewer::get_by_machinename($machine_name);
    if ($row) {
      unset($row['nid'], $row['vid'], $row['changed'], $row['created']);
      $export = features_var_export($row, '  ');
      $code[] = "  // Exported mediamosa viewer: {$row['server_name']}.";
      $code[] = "  \$" . FEATURE_MEDIAMOSA_VIEWER . "s['{$machine_name}'] = {$export};";
    }
  }

  $code[] = '';
  $code[] = '  return $' . FEATURE_MEDIAMOSA_VIEWER . 's;';
  $code = implode("\n", $code);

  return array(FEATURE_MEDIAMOSA_VIEWER . 's' => $code);
}

/**
 * Implements hook_features_revert().
 */
function mediamosa_viewer_features_revert($module_name) {
  $mediamosa_viewers = features_get_default(FEATURE_MEDIAMOSA_VIEWER, $module_name);

  $fields = mediamosa_viewer_db::get_fields();
  foreach ($mediamosa_viewers as $machine_name => $mediamosa_viewer) {
    $mediamosa_viewer_loaded = mediamosa_viewer::get_by_machinename($machine_name);
    if (empty($mediamosa_viewer_loaded)) {
      // Create new server.
      $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_VIEWER, $mediamosa_viewer_loaded[mediamosa_viewer_db::TOOL]);
    }
    else {
      $node = node_load($mediamosa_viewer_loaded['nid']);
    }

    foreach ($mediamosa_viewer as $field => $value) {
      if (in_array($field, $fields)) {
        $node->{$field} = $value;
      }
    }

    // We copy name of the viewer to the node title in our node code.
    $node->title = $mediamosa_viewer[mediamosa_viewer_db::NAME];

    node_save($node);
  }
}
