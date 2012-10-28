<?php
/**
 * @file
 * Installer metadata set.
 */

require_once 'asset/mediamosa_metadata_asset.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_metadata_install() {
  _mediamosa_metadata_asset_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_metadata_uninstall() {
  _mediamosa_metadata_asset_uninstall();
}
