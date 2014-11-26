<?php
/**
 * @file
 * DB classes for user_group relationship.
 */

class mediamosa_user_group_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_user_group';

  // Primary KEY
  const KEY = 'group_key';

  // Database fields;
  const ID = 'group_id';
  const ID_LENGTH = 255;
  const GROUP_ID_LENGTH = 255;
  const APP_ID = 'app_id';
  const QUOTUM = 'quotum';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
