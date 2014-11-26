<?php
/**
 * @file
 * User DB Class.
 */

class mediamosa_user_db {
  // ------------------------------------------------------------------ Consts.
  const TABLE_NAME = 'mediamosa_user';

  // Primary KEY
  const KEY = 'user_key';

  // Database fields;
  const APP_ID = 'app_id';
  const ID = 'name';
  const NAME = 'name';
  const NAME_LENGTH = 255;

  // Obsoleted, not in use anymore.
  const GROUP_ID = 'group_id';
  const GROUP_ID_LENGTH = 255;

  const QUOTUM = 'quotum';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
