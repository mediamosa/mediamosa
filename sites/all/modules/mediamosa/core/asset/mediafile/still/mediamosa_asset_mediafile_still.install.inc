<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * The install file for asset media
 */

require_once 'mediamosa_asset_mediafile_still.class.inc';
require_once 'style/mediamosa_asset_mediafile_still_style.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_mediafile_still_install() {
  // Call the asset media still style installer.
  _mediamosa_asset_mediafile_still_style_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_mediafile_still_uninstall() {
  // Call the asset media still style uninstaller.
  _mediamosa_asset_mediafile_still_style_uninstall();
}
