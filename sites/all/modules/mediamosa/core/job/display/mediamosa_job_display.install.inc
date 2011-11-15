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
 * Install for mediamosa_job_display table.
 */
require_once 'mediamosa_job_display_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_job_display_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_job_display_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_job_display_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_job_display_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_job_display_schema() {

  $schema[mediamosa_job_display_db::TABLE_NAME] = array(
  'description' => 'The job display table.',
    'fields' => array(
      mediamosa_job_display_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_job_display_db::VISIBILITY => array(
        'description' => 'The visibility attribute.',
        'type' => "enum('" . implode("','", array(mediamosa_job_display_db::VISIBILITY_DISPLAY, mediamosa_job_display_db::VISIBILITY_HIDDEN)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_display_db::VISIBILITY_DISPLAY, mediamosa_job_display_db::VISIBILITY_HIDDEN)) . "')",
        'default' => mediamosa_job_display_db::VISIBILITY_DISPLAY,
      ),
    ),
    'primary key' => array(mediamosa_job_display_db::ID),
  );

  return $schema;
}
