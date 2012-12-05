<?php
/**
 * @file
 * The database class of the asset module.
 */

class mediamosa_collection_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_collection'; // 1.x name collection.

  // Database fields;
  const ID = 'coll_id';
  const APP_ID = 'app_id';
  const TITLE = 'title';
  const TITLE_LENGTH = 255;
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 1000;
  const OWNER_ID = 'owner_id';
  const OWNER_ID_LENGTH = 255;
  const GROUP_ID = 'group_id';
  const GROUP_ID_LENGTH = 255;
  const ISPRIVATE = 'isprivate';
  const ISPRIVATE_TRUE = 'TRUE';
  const ISPRIVATE_FALSE = 'FALSE';
  const _PRIVATE = 'private';
  const PRIVATE_ = 'private';
  const PRIVATE_TRUE = 'TRUE';
  const PRIVATE_FALSE = 'FALSE';
  const _PUBLIC = 'public';
  const PUBLIC_ = 'public';
  const PUBLIC_TRUE = 'TRUE';
  const PUBLIC_FALSE = 'FALSE';
  const CATEGORY = 'category';
  const CATEGORY_TRUE = 'TRUE';
  const CATEGORY_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const PUBLIC_ASSIGN = 'public_assign';
  const PUBLIC_ASSIGN_TRUE = 'TRUE';
  const PUBLIC_ASSIGN_FALSE = 'FALSE';
  const IS_INAPPROPRIATE = 'is_unappropriate';
  const IS_INAPPROPRIATE_TRUE = 'TRUE';
  const IS_INAPPROPRIATE_FALSE = 'FALSE';
}
