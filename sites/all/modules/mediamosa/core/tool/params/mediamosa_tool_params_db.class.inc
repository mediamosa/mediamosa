<?php
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
  const MACHINE_NAME = 'machine_name';
  const MACHINE_NAME_LENGTH = 255;
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
  const WEIGHT = 'weight';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_tool_params.install.inc';

    // Get the schema.
    $a_schema = mediamosa_tool_params_schema();
    return array_keys($a_schema[mediamosa_tool_params_db::TABLE_NAME]['fields']);
  }
}
