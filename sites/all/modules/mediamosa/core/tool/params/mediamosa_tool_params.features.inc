<?php
/**
 * @file
 * Core - Features export and import hooks for MediaMosa tool params objects.
 */

/**
 * Implements hook_features_export_options().
 */
function mediamosa_tool_params_features_export_options() {
  $options = array();

  $tool_params = mediamosa_tool_params::search();
  foreach ($tool_params as $tool_param) {
    $options[$tool_param[mediamosa_tool_params_db::TOOL]] = $tool_param[mediamosa_tool_params_db::TOOL];
  }

  return $options;
}

/**
 * Implements hook_features_export().
 */
function mediamosa_tool_params_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ($data as $tool) {
    $export['dependencies']['mediamosa'] = 'mediamosa';
    $export['features'][FEATURE_MEDIAMOSA_TOOL_PARAMS][$tool] = $tool;
  }
  return $pipe;
}

/**
 * Implements hook_features_export_render()
 */
function mediamosa_tool_params_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $' . FEATURE_MEDIAMOSA_TOOL_PARAMS . ' = array();';
  $code[] = '';
  foreach ($data as $tool) {
    $params = mediamosa_tool_params::get_by_tool($tool);
    foreach ($params as $row) {
      if ($row) {
        unset($row['nid'], $row['vid'], $row['tool_params_id'], $row['changed']);
        $export = features_var_export($row, '  ');
        $code[] = "  // Exported mediamosa tool {$row[mediamosa_tool_params_db::TOOL]} params: {$row[mediamosa_tool_params_db::NICE_PARAMETER]}.";
        $code[] = '  $'  . FEATURE_MEDIAMOSA_TOOL_PARAMS .  "['{$row[mediamosa_tool_params_db::MACHINE_NAME]}'] = {$export};";
      }
    }
  }

  $code[] = '';
  $code[] = '  return $' . FEATURE_MEDIAMOSA_TOOL_PARAMS . ';';
  $code = implode("\n", $code);

  return array(FEATURE_MEDIAMOSA_TOOL_PARAMS => $code);
}

/**
 * Implements hook_features_revert().
 *
 * @param string $module_name
 *   The name of the feature module whose components should be reverted.
 */
function mediamosa_tool_params_features_revert($module_name) {
  $mediamosa_tool_paramss = features_get_default(FEATURE_MEDIAMOSA_TOOL_PARAMS, $module_name);

  $fields = mediamosa_tool_params_db::get_fields();
  foreach ($mediamosa_tool_paramss as $machine_name => $mediamosa_tool_params) {
    $mediamosa_tool_params_loaded = mediamosa_tool_params::get_by_machinename($machine_name);
    if (empty($mediamosa_tool_params_loaded)) {
      // Create new server.
      $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS, $mediamosa_tool_params_loaded[mediamosa_tool_params_db::TOOL]);
   }
    else {
      $node = node_load($mediamosa_tool_params_loaded['nid']);
    }

    foreach ($mediamosa_tool_params as $field => $value) {
      if (in_array($field, $fields)) {
        $node->{$field} = $value;
      }
    }

    // We copy the node title to tool name in our node code.
    $node->title = $mediamosa_tool_params[mediamosa_tool_params_db::NICE_PARAMETER];

    node_save($node);
  }
}
