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
  * The installer for Authorization modules.
  */

require_once 'name/mediamosa_aut_name.install.inc';
require_once 'group/mediamosa_aut_group.install.inc';
require_once 'object/mediamosa_aut_object.install.inc';
require_once 'app_master_slave/mediamosa_aut_app_master_slave.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_aut_install() {

  // Install mediamosa_aut_name.
  _mediamosa_aut_name_install();

  // Install mediamosa_aut_group.
  _mediamosa_aut_group_install();

  // Install mediamosa_aut_object.
  _mediamosa_aut_object_install();

  // Install mediamosa_aut_app_master_slave.
  _mediamosa_aut_app_master_slave_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_aut_uninstall() {
  // Install mediamosa_aut_app_master_slave.
  _mediamosa_aut_app_master_slave_uninstall();

  // Install mediamosa_aut_object.
  _mediamosa_aut_object_uninstall();

  // Install mediamosa_aut_group.
  _mediamosa_aut_group_uninstall();

  // Install mediamosa_aut_name.
  _mediamosa_aut_name_uninstall();
}
