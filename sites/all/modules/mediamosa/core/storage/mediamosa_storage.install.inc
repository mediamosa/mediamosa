<?php
/**
 * @file
 * Installer Storage.
 */

require_once 'local/mediamosa_storage_local.install.inc';
require_once 'profile/mediamosa_storage_profile.install.inc';
require_once 'profile/app/mediamosa_storage_profile_app.install.inc';
require_once 'transition/mediamosa_storage_transition.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_storage_install() {
  // Create our table(s).
  _mediamosa_storage_transition_install();
  _mediamosa_storage_profile_install();
  _mediamosa_storage_profile_app_install();
  _mediamosa_storage_local_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_uninstall() {
  // Drop our table(s).
  _mediamosa_storage_local_uninstall();
  _mediamosa_storage_profile_app_uninstall();
  _mediamosa_storage_profile_uninstall();
  _mediamosa_storage_transition_uninstall();
}
