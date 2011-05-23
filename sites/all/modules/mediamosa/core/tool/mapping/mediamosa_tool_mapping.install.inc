<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * Tool mapping installer.
  */

require_once 'mediamosa_tool_mapping_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_tool_mapping_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_tool_mapping_db::TABLE_NAME);

  // Create our node type.
  $tool_mapping_node_type = array(
    'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_MAPPING,
    'name' => st('MediaMosa tool mapping'),
    'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_MAPPING,
    'description' => st('The tool mapping contains information about how to map the tool (eg. pdf, tiff) analyse data to asset metadata.'),
    'custom' => 1,
    'modified' => 1,
    'locked' => 0,
    'has_body' => 0,
  );

  $tool_mapping_node_type = node_type_set_defaults($tool_mapping_node_type);
  node_type_save($tool_mapping_node_type);
}

/**
 * Implements hook_unstall().
 */
function _mediamosa_tool_mapping_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_tool_mapping_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_tool_mapping_schema() {

  $schema[mediamosa_tool_mapping_db::TABLE_NAME] = array(
  'description' => 'The tool mapping table.',
    'fields' => array(
      mediamosa_tool_mapping_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_tool_mapping_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_tool_mapping_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_tool_mapping_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_mapping_db::TOOL_LENGTH,
        'description' => 'Tool.',
      ),
      mediamosa_tool_mapping_db::PARAMETER => array(
        'type' => 'varchar',
        'length' => mediamosa_tool_mapping_db::PARAMETER_LENGTH,
        'description' => 'Parameter (name of the parameter).',
      ),
      mediamosa_tool_mapping_db::PROP_ID => array(
        'type' => 'int',
        'description' => 'The asset metadata property id.',
        'default' => NULL,
      ),
      mediamosa_tool_mapping_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when tool mapping was created.',
      ),
      mediamosa_tool_mapping_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when tool mapping was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_tool_mapping_db::ID),
    'unique keys' => array(
      'uni_niceparam_tool' => array(mediamosa_tool_mapping_db::PARAMETER, mediamosa_tool_mapping_db::TOOL),
      'uni_nid' => array(mediamosa_tool_mapping_db::NID),
    ),
    'indexes' => array(
      'idx_tool' => array(mediamosa_tool_mapping_db::TOOL),
    )
  );

  return $schema;
}
