<?php
/**
 * @file
 * Install, update, and uninstall functions for the MediaMosa module.
 */

require_once 'ticket/mediamosa_media_ticket.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_media_install() {
  // Install media ticket.
  _mediamosa_media_ticket_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_media_uninstall() {
  // Uninstall media ticket.
  _mediamosa_media_ticket_uninstall();
}
