<?php
/**
 * @file
 * The DB ticket class.
 */

class mediamosa_media_ticket_db {

  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_media_ticket';

  // Database fields;
  const ID = 'ticket_id';
  const TICKET_TYPE = 'ticket_type';
  const TICKET_TYPE_VIEW = 'PLAY';
  const TICKET_TYPE_DOWNLOAD = 'DOWNLOAD';
  const TICKET_TYPE_STILL = 'STILL';
  const TICKET_TYPE_UPLOAD = 'UPLOAD';
  const ISSUED = 'issued';
  const APP_ID = 'app_id';
  const OWNER_ID = 'owner_id';
  const OWNER_ID_LENGTH = 255;
  const GROUP_ID = 'group_id';
  const GROUP_ID_LENGTH = 255;
  const MEDIAFILE_ID = 'mediafile_id';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
