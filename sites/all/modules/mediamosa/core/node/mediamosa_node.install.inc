<?php
/**
 * @file
 * Dummy file for calling mediamosa_node_revision installer.
 */

require_once 'revision/mediamosa_node_revision.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_node_install() {

  // Installer node revision.
  _mediamosa_node_revision_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_node_uninstall() {

  // Installer node revision.
  _mediamosa_node_revision_uninstall();
}
