<?php
/**
 * @file
 * Error database class object.
 */

class mediamosa_error_db {
  // ------------------------------------------------------------------- Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_error';

  // Database fields;
  const ID = 'error_id';
  const NAME = 'name';
  const NAME_LENGTH = 100;
  const CODE = 'code';
  const CODE_RANGE_START = 1;
  const CODE_RANGE_END = 9999;
  const MESSAGE = 'message';
}
