<?php
/**
 * @file
 * Installer for mediamosa_server_streaming module.
 */

require_once 'container/mediamosa_server_streaming_container.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_streaming_install() {

  // Our child.
  _mediamosa_server_streaming_container_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_streaming_uninstall() {

  // Our child.
  _mediamosa_server_streaming_container_uninstall();
}
