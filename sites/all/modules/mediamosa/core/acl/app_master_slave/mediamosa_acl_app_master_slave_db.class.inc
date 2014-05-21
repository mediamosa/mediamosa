<?php
/**
 * @file
 * DB description of mediamosa_acl_app_master_slave table.
 */

class mediamosa_acl_app_master_slave_db {
  // ------------------------------------------------------------------- Consts.
  const TABLE_NAME = 'mediamosa_acl_app_master_slave';

  const ACL_OBJECT_ID = 'acl_object_id';
  const ACL_OBJECT_ID_LENGTH = 32;
  const ACL_OBJECT_TYPE = 'acl_object_type';
  const ACL_OBJECT_TYPE_MEDIAFILE = 'MEDIAFILE';
  const ACL_OBJECT_TYPE_ASSET = 'ASSET';
  const ACL_OBJECT_TYPE_COLLECTION = 'COLLECTION';

  const APP_ID_SLAVE = 'app_id_slave';
  const APP_ID_MASTER = 'app_id_master';
}
