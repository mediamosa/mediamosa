<?php
/**
 * @file
 * The asset collection DB contains the relation links between the assets and
 * collections.
 */

class mediamosa_asset_collection_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_collection';

  // Database fields;
  const ASSET_ID = 'asset_id';
  const COLL_ID = 'coll_id';
  const IS_EMPTY_ASSET = 'is_empty_asset';
  const IS_EMPTY_ASSET_TRUE = 'TRUE';
  const IS_EMPTY_ASSET_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
