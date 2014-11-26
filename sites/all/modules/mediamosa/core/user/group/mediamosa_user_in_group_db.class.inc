<?php
/**
 * @file
 * DB classes for user/user_group relationship.
 */

class mediamosa_user_in_group_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_user_in_group';

  // Primary KEY
  const ID = 'relation_id';

  // Relation
  const USER_KEY = 'user_key';
  const GROUP_KEY = 'group_key';
}
