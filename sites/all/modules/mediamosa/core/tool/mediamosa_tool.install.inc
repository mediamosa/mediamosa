<?php
/**
 * @file
 * Tool installer.
 */

require_once 'params/mediamosa_tool_params.install.inc';
require_once 'mapping/mediamosa_tool_mapping.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_tool_install() {
  _mediamosa_tool_params_install();
  _mediamosa_tool_mapping_install();
}

/**
 * Implements hook_unstall().
 */
function _mediamosa_tool_uninstall() {
  _mediamosa_tool_params_uninstall();
  _mediamosa_tool_mapping_uninstall();
}
