<?php
/**
 * @file
 * The database class of the asset_mediafile metadata module.
 */

class mediamosa_asset_mediafile_metadata_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_mediafile_metadata';

  // Database fields;
  const ID = 'metadata_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const PROP_ID = 'prop_id';
  const VAL_CHAR = 'val_char';
  const VAL_DATETIME = 'val_datetime';
  const VAL_INT = 'val_int';
}
