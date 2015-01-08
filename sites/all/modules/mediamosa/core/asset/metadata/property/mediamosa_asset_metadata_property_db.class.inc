<?php
/**
 * @file
 * The database class of the asset module.
 */

class mediamosa_asset_metadata_property_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_metadata_property';

  // Database fields;
  const ID = 'prop_id';
  const PROPGROUP_ID = 'propgroup_id';
  const APP_ID = 'app_id';
  const NAME = 'prop_name';
  const NAME_LENGTH = 32;
  const TYPE = 'type';
  const TYPE_DATETIME = 'DATETIME';
  const TYPE_INT = 'INT';
  const TYPE_CHAR = 'CHAR';
  const MIN_OCCURRENCES = 'min_occurrences';
  const MAX_OCCURRENCES = 'max_occurrences';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const IS_HIDDEN = 'is_hidden';
  const IS_HIDDEN_TRUE = 'TRUE';
  const IS_HIDDEN_FALSE = 'FALSE';
  const IS_PROTECTED = 'is_protected';
  const IS_PROTECTED_TRUE = 'TRUE';
  const IS_PROTECTED_FALSE = 'FALSE';
}
