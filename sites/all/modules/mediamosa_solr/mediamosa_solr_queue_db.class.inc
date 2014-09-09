<?php
/**
 * @file
 * Database file for mediamosa_solr.
 */

class mediamosa_solr_queue_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_solr_queue';

  // Database fields;
  const ID = 'id';  // For now its always the asset_id.
  const CREATED = 'created';
}
