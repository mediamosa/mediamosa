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
 * The tool mapping DB class.
 */
class mediamosa_tool_mapping_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_tool_mapping';

  // Database fields;
  const NID = 'nid';
  const VID = 'vid';
  const ID = 'tool_mapping_id';
  const TOOL = 'tool';
  const TOOL_LENGTH = 50;
  const PARAMETER = 'parameter';
  const PARAMETER_LENGTH = 50;
  const PROP_ID = 'prop_id';
  const CREATED = 'created';
  const CHANGED = 'changed';

  /**
   * Returns all fields.
   */
  static public function get_fields() {
    require_once 'mediamosa_tool_mapping.install.inc';

    // Get the schema.
    $a_schema = mediamosa_tool_mapping_schema();
    return array_keys($a_schema[mediamosa_tool_mapping_db::TABLE_NAME]['fields']);
  }
}
