<?php
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
  public static function get_fields() {
    require_once 'mediamosa_tool_mapping.install.inc';

    // Get the schema.
    $a_schema = mediamosa_tool_mapping_schema();
    return array_keys($a_schema[mediamosa_tool_mapping_db::TABLE_NAME]['fields']);
  }
}
