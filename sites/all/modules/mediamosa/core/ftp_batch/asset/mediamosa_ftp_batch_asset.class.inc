<?php
/**
 * @file
 * The ftp batch asset functions.
 */

class mediamosa_ftp_batch_asset {

  /**
   * Get the asset relations.
   *
   * @param int $app_id
   *  The application ID.
   * @param int $batch_id
   *  The batch ID.
   * @param int $offset
   *  The offset in the result set.
   * @param int $limit
   *  The limit of items in the result set.
   */
  public static function get_asset_relations($app_id, $batch_id, $offset, $limit) {

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'asset_id';
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_ftp_batch_asset}';

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = "batch_id=" . $batch_id;
    $a_query[mediamosa_db_query::A_ORDER_BY][] = "asset_id ASC";

    $a_query[mediamosa_db_query::LIMIT] = $limit;
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Build the query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => TRUE));

    // Do the query.
    $result = mediamosa_db::db_query($query);

    // Set total found rows.
    $result->found_rows = mediamosa_db::db_found_rows();

    return $result;
  }

  /**
   * Put assets in a batch.
   *
   * @param int $app_id
   *  Application ID.
   * @param array $asset_ids
   *  The asset IDs.
   * @param int $batch_id
   *  The batch ID.
   */
  public static function add_asset_relation($app_id, array $asset_ids, $batch_id) {

    // Make sure all assets exist.
    foreach ($asset_ids as $asset_id) {
      mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id, mediamosa_asset_db::APP_ID => $app_id));
    }

    // Now create relations.
    foreach ($asset_ids as $asset_id) {
      try {
        // Add this asset as row in {ftp_batch_asset} with [$asset_id, $batch_id].
        mediamosa_db::db_query('INSERT INTO {#mediamosa_ftp_batch_asset} SET #asset_id = :asset_id, #batch_id = :batch_id',
          array(
            '#mediamosa_ftp_batch_asset' => mediamosa_ftp_batch_asset_db::TABLE_NAME,
            '#batch_id' => mediamosa_ftp_batch_asset_db::BATCH_ID,
            ':batch_id' => $batch_id,
            '#asset_id' => mediamosa_ftp_batch_asset_db::ASSET_ID,
            ':asset_id' => $asset_id
          )
        );
      }
      catch (PDOException $e) {
        assert($e);
        // ignore. failed because its already in the database.
      }
    }
  }

  /**
   * Remove all relations of one ftp batch.
   *
   * @param int $batch_id
   *  ID of batch to delete.
   */
  public static function remove_asset_relation($batch_id) {

    // Delete batch asset relations.
    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_ftp_batch_asset} WHERE #batch_id = :batch_id',
      array(
        '#mediamosa_ftp_batch_asset' => mediamosa_ftp_batch_asset_db::TABLE_NAME,
        '#batch_id' => mediamosa_ftp_batch_asset_db::ID,
        ':batch_id' => $batch_id
      )
    );
  }

  /**
   * Get the batches of given asset.
   *
   * @param string $asset_id
   */
  public static function get_batch_relation($asset_id) {
    return mediamosa_db::db_select(mediamosa_ftp_batch_asset_db::TABLE_NAME, 'ftp')
      ->fields('ftp', array(mediamosa_ftp_batch_asset_db::BATCH_ID))
      ->condition(mediamosa_ftp_batch_asset_db::ASSET_ID, $asset_id)
      ->execute()
      ->fetchCol();
  }
}
