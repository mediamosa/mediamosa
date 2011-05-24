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
 * The tool params DB class.
 */
class mediamosa_tool_params_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_tool_params';

  // Database fields;
  const NID = 'nid';
  const VID = 'vid';
  const ID = 'tool_params_id';
  const TOOL = 'tool';
  const TOOL_LENGTH = 50;
  const NICE_PARAMETER = 'nice_parameter';
  const NICE_PARAMETER_LENGTH = 50;
  const TOOL_PARAMETER = 'tool_parameter';
  const TOOL_PARAMETER_LENGTH = 50;
  const MIN_VALUE = 'min_value';
  const MAX_VALUE = 'max_value';
  const ALLOWED_VALUE = 'allowed_value';
  const ALLOWED_VALUE_LENGTH = 1000;
  const ALLOWED_VALUE_FOR_SWITCH = '<switch>';
  const DEFAULT_VALUE = 'default_value';
  const DEFAULT_VALUE_LENGTH = 50;
  const REQUIRED = 'required';
  const REQUIRED_TRUE = 'TRUE';
  const REQUIRED_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const DESCRIPTION = 'description';
  const TYPE_PARAMETER = 'type_parameter';
  const TYPE_PARAMETER_CHECKBOX = 'CHECKBOX';
  const TYPE_PARAMETER_SELECT = 'SELECT';

  /**
   * Returns all fields.
   */
  static public function get_fields() {
    require_once 'mediamosa_tool_params.install.inc';

    // Get the schema.
    $a_schema = mediamosa_tool_params_schema();
    return array_keys($a_schema[mediamosa_tool_params_db::TABLE_NAME]['fields']);
  }
}
