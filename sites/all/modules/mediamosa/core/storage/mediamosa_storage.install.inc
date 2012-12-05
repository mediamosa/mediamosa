<?php
// $Id$

/**
 * @file
 * Installer Storage.
 */
require_once 'profile/mediamosa_storage_profile.install.inc';
require_once 'mapping/mediamosa_storage_mapping.install.inc';
require_once 'standard/mediamosa_storage_standard.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_storage_install() {
  // Create our table(s).
  _mediamosa_storage_profile_install();
  _mediamosa_storage_mapping_install();
  _mediamosa_storage_standard_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_uninstall() {
  // Drop our table(s).
  _mediamosa_storage_standard_uninstall();
  _mediamosa_storage_mapping_uninstall();
  _mediamosa_storage_profile_uninstall();
}
