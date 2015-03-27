<?php
/**
 * @file
 * The MediaMosa viewer class.
 */

class mediamosa_viewer {
  // ------------------------------------------------------------------- Consts.

  // ---------------------------------------------------------------- Functions.
  /**
   * Get all viewers.
   */
  public static function search() {
    return mediamosa_db::db_select(mediamosa_viewer_db::TABLE_NAME, 't')
      ->fields('t')
      ->execute();
  }

  /**
   * Get the viewer using its machine name.
   *
   * @param string $machine_name
   *   The machine name.
   *
   * @return array
   *   The MediaMosa viewer or FALSE.
   */
  public static function get_by_machinename($machine_name) {
    return mediamosa_db::db_select(mediamosa_viewer_db::TABLE_NAME, 't')
      ->fields('t')
      ->condition(mediamosa_viewer_db::MACHINE_NAME, $machine_name)
      ->execute()
      ->fetchAssoc();
  }
}
