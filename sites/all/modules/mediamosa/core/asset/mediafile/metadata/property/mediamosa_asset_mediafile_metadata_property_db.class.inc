<?php
/**
 * @file
 * The database class of the asset_mediafile metadata module.
 */

class mediamosa_asset_mediafile_metadata_property_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_mediafile_metadata_property';

  // Database fields;
  const ID = 'prop_id';
  const NAME = 'prop_name';
  const NAME_LENGTH = 32;
  const TYPE = 'type';
  const TYPE_DATETIME = 'DATETIME';
  const TYPE_INT = 'INT';
  const TYPE_CHAR = 'CHAR';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
