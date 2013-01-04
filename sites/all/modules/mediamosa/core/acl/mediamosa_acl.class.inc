<?php
/**
 * @file
 * Authorization layer for assets, media functions.
 */

class mediamosa_acl {

  // ------------------------------------------------------------------ Consts.
  const ACL_TYPE_MEDIAFILE = 'MEDIAFILE';
  const ACL_TYPE_ASSET = 'ASSET';
  const ACL_TYPE_COLLECTION = 'COLLECTION';

  // Current rights
  const RIGHT_ACCESS = 'ACCESS';

  const ACL_NAME_TYPE_DOMAIN = 'DOMAIN';
  const ACL_NAME_TYPE_REALM = 'REALM';
  const ACL_NAME_TYPE_USER = 'USER';
  const ACL_NAME_TYPE_USER_GROUP = 'USER_GROUP';

  // Dummy for old ACL layer.
  const ACL_NAME_TYPE_APP = 'APP';

  // Dummy for result rights, name and PATH.
  const OBJ_LINK_TYPE_APP = 'APP';
  const OBJ_LINK_TYPE_NAME = 'NAME';
  const OBJ_LINK_TYPE_GROUP = 'GROUP';

  // Copies from the table asset_mediafile column is_protected enum.
  const MEDIAFILE_IS_PROTECTED_FALSE = 'FALSE';
  const MEDIAFILE_IS_PROTECTED_TRUE = 'TRUE';
  const MEDIAFILE_IS_PROTECTED_USER_USERGROUP = 'USER_USERGROUP';
  const MEDIAFILE_IS_PROTECTED_DOMAIN_REALM = 'DOMAIN_REALM';

  // ------------------------------------------------------------------ Static Function.
  public static function valid_realm($realm) {
    assert($realm != '');
    return TRUE;
  }

  public static function valid_domain($domain) {
    assert($domain != '');
    return TRUE;
  }

  /**
   * Test the format of the group domain name.
   *
   * @param string $group_domain
   *   The group domain to check.
   *
   * @return boolean
   *   Returns TRUE when valid group domain.
   */
  public static function is_group_domain($group_domain) {
    return (!self::is_group_realm($group_domain) && preg_match('/^[^.@]+\.group$/', $group_domain) > 0 ? TRUE : FALSE);
  }

  /**
   * Test the format of the group realm name.
   *
   * @param string $group_realm
   *   The realm group to check.
   *
   * @return boolean
   *   Returns TRUE when valid group realm.
   */
  public static function is_group_realm($group_realm) {
    return (preg_match('/^@[^.]+\.group$/', $group_realm) > 0 ? TRUE : FALSE);
  }

  // Simple check if input is realm
  public static function is_realm($s_hostname) {
    return ((strpos($s_hostname, '@') === FALSE) ? FALSE : TRUE);
  }

  /**
   * Returns the acl_ids of the given domain and realms.
   *
   * @param integer/array $mixed_app_id
   * @param string $acl_user_id
   * @param array $a_acl_group_id
   *  These are user_id groups, like mf.group_id
   * @param array $acl_domain
   * @param array $acl_realm
   */
  public static function build_access($mixed_app_id, $acl_user_id = NULL, $acl_group_ids = NULL, $acl_domain = NULL, $acl_realm = NULL, $a_slaves = NULL) {
    $a_query = array();

    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_acl_name}.acl_name_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_acl_name}.acl_group_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_acl_name}.acl_type';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_acl_name}.app_id';

    // Make it an array.
    if (!is_array($mixed_app_id)) {
      $mixed_app_id = array($mixed_app_id);
    }

    // Prevent mysql choosing the uni index above the needed index.
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_acl_name}';

    assert(is_array($acl_group_ids) || is_null($acl_group_ids));

    // Make sure some value's are set to NULL when considered empty.
    $acl_group_ids = empty($acl_group_ids) ? NULL : $acl_group_ids;
    $acl_domain = $acl_domain == '' ? NULL : $acl_domain;
    $acl_realm = $acl_realm == '' ? NULL : $acl_realm;

    // Make sure we do at least one of the wheres besides app_id, or else we will have access on all.
    $empty_access = TRUE;

    // First the app_id.
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['app_id'][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_acl_name}.app_id IN(%s)", implode(",", $mixed_app_id));

    // Do we have entry for our user_id and is not in a group (future exp. but lets do it now anyway)
    if (!is_null($acl_user_id) && $acl_user_id != "") {
      $empty_access = FALSE;

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['user'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_type = 'USER'";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['user'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_group_id IS NULL";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['user'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_acl_name}.acl_name = '%s'", mediamosa_db_query::query_escape($acl_user_id));
    }

    // Do we have entry for our user_id and is not in a group (group type is never in a group, we will not group groups here)
    if (!is_null($acl_group_ids) && !empty($acl_group_ids)) {
      // Make sure there are no 'empty' group_ids in our list, or else we will gain access by mistake
      foreach ($acl_group_ids as $key => $acl_group_id) {
        if (trim($acl_group_id) == "") {
          unset($acl_group_ids[$key]);
        }
      }

      if (!empty($acl_group_ids)) {
        $empty_access = FALSE;

        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_type = 'USER_GROUP'";
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_group_id IS NULL";

        if (count($acl_group_ids) == 1) {
            $acl_group_id = reset($acl_group_ids);
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_acl_name}.acl_name = '%s'", mediamosa_db_query::query_escape($acl_group_id));
        }
        else {
          foreach ($acl_group_ids as $acl_group_id) {
            if (!empty($acl_group_id)) {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND]['group'][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_acl_name}.acl_name = '%s'", mediamosa_db_query::query_escape($acl_group_id));
            }
          }
        }
      }
    }

    // Does my domain match with any in the listing?
    if (!is_null($acl_domain)) {
      $empty_access = FALSE;

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_type='DOMAIN'";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND]['acl_group_id'][mediamosa_db_query::WHERE_OR][] = "{mediamosa_acl_name}.acl_group_id IS NULL";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND]['acl_group_id'][mediamosa_db_query::WHERE_OR][] = "{mediamosa_acl_name}.acl_group_id IS NOT NULL";

      // Split the domain on its '.'.
      $a_domain_parts = explode('.', $acl_domain);

      // Now reverse it.
      $a_domain_parts = array_reverse($a_domain_parts);

      // Now walk through and each time add one more.
      $a_domain_parts_to_join = array(reset($a_domain_parts));

      while (1) {
        $domain_part = next($a_domain_parts);

        $acl_domain = implode('.', array_reverse($a_domain_parts_to_join));
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND]['acl_name'][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_acl_name}.acl_name = '%s'", mediamosa_db_query::query_escape($acl_domain));

        if ($domain_part === FALSE) {
          break; // from while.
        }

        $a_domain_parts_to_join[] = $domain_part; // next part plz.
      }

      // Domain type does not use prefix field so, we dont need to test IS NULL either.
    }

    if (!is_null($acl_realm)) {
      $empty_access = FALSE;

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_type='REALM'";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND]["acl_group_id"][mediamosa_db_query::WHERE_OR][] = "{mediamosa_acl_name}.acl_group_id IS NULL";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND]["acl_group_id"][mediamosa_db_query::WHERE_OR][] = "{mediamosa_acl_name}.acl_group_id IS NOT NULL";

      // Realm check is special, very special

      // Must contain @, if not, include it
      if (strpos($acl_realm, '@') === FALSE) {
        $acl_realm = '@' . $acl_realm;
      }

      list($acl_prefix, $s_acl_domain) = explode('@', trim($acl_realm));
      if (trim($acl_prefix) == "") {
        $acl_prefix = NULL;
      }

      // do the same thing as we did on domain

      // Split the domain on its '.'
      $a_domain_parts = explode('.', $s_acl_domain);

      // Now reverse it
      $a_domain_parts = array_reverse($a_domain_parts);

      // Now walk through and each time add one more
      $a_domain_parts_to_join = array(reset($a_domain_parts));

      while (1) {
        $acl_realm = '@' . implode('.', array_reverse($a_domain_parts_to_join));

        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['acl_name'][mediamosa_db_query::WHERE_AND]["acl_prefix"][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_acl_name}.acl_name = '%s'", mediamosa_db_query::query_escape($acl_realm));

        $domain_part = next($a_domain_parts);
        if ($domain_part === FALSE) {
          break; // from while
        }

        $a_domain_parts_to_join[] = $domain_part;
      }

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['acl_name'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_acl_name}.acl_prefix IS NULL";

      if (!is_null($acl_prefix)) {
        // if prefix was given we also check on the full [name] @ [domain] access
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['acl_name_full'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_acl_name}.acl_name = '%s'", mediamosa_db_query::query_escape('@' . $s_acl_domain));// its a realm, even though its called domain in its var name
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['acl_name_full'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_acl_name}.acl_prefix = '%s'", mediamosa_db_query::query_escape($acl_prefix));
      }
    }

    // If no access was checked with where, then we have no access on any protected object.
    if ($empty_access) {
      return array();
    }

    // Build the query.
    $query_sql = mediamosa_db_query::query_select($a_query);

    // Execute the query.
    $db_result = mediamosa_db::db_query($query_sql);

    // Our result array.
    $a_result = array();

    // Create empty result for each app.
    foreach ($mixed_app_id as $app_id) {
      $a_result[$app_id] = array('a_name' => array(), 'a_group' => array());
    }

    foreach ($db_result as $a_row) {
      if (intval($a_row['acl_group_id'])) {
        $a_result[$a_row['app_id']]['a_group'][] = $a_row['acl_group_id'];
      }
      else {
        $a_result[$a_row['app_id']]['a_name'][] = $a_row['acl_name_id'];
      }
    }

    $a_groups = array();
    foreach ($a_result as $app_id => $a_row) {
      // If no acl ids where found for this app, then remove it and continue.
      if (!count($a_row['a_group']) && !count($a_row['a_name'])) {
        unset($a_result[$app_id]);
        continue;
      }

      // make them unique.
      array_unique($a_result[$app_id]['a_group']);
      array_unique($a_result[$app_id]['a_name']);

      // Collect all groups from each app.
      $a_groups = array_merge($a_groups, $a_result[$app_id]['a_group']);

      // sort them, is a bit faster for query
      sort($a_result[$app_id]['a_group'], SORT_NUMERIC);
      sort($a_result[$app_id]['a_name'], SORT_NUMERIC);
    }

    // Make them unique.
    array_unique($a_groups);

    /**
     * The main app_id has domains / realms. These might also be on the other apps (a_slaves).
     *
     * f.e.
     * academia5.group has foo.nl domain in app 3.
     * academia5.group has surfnet.nl domain in app 5.
     * The app3 is mastered to app5 (mediafile = app3) and Im app5 with domain 'surfnet.nl'
     * I may have access to mf (app 3) if my group app5 academia5.group (because of surfnet.nl) also is on app3.
     * If surfnet.nl is under academia5.group app3 and not under academia5.group app5 (my app), then I dont have access.
     *
     */
    if (isset($a_slaves) && count($a_slaves) && count($a_groups)) {
      /**
       * $a_slaves - contains all app_ids that are master-slaved to me.
       * $a_groups - contains all group ids of all the groups of all apps based on my acl settings.
       */

      // Collect all names.
      $query = mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag');
      $query->fields('ag', array(
        mediamosa_acl_group_db::ID,
        mediamosa_acl_group_db::ACL_GROUP_NAME,
        mediamosa_acl_group_db::ACL_GROUP_TYPE
      ));
      $query->condition(mediamosa_acl_group_db::ID, $a_groups, 'IN');

      /*
       * Now with all group names + types, find the groups of the other apps.
       */
      foreach ($query->execute() as $a_group) {
        $query_2 = mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag');
        $query_2->fields('ag', array(
          mediamosa_acl_group_db::APP_ID,
          mediamosa_acl_group_db::ID
        ));
        $query_2->condition(mediamosa_acl_group_db::ACL_GROUP_NAME, $a_group['acl_group_name']);
        $query_2->condition(mediamosa_acl_group_db::ACL_GROUP_TYPE, $a_group['acl_group_type']);

        // Loop through the results of muy group name + type search, so we find the groups of all apps.
        foreach ($query_2->execute() as $a_group_2) {
          // If app of found group is not one of my slaves, then I cant use it.
          if (!isset($a_slaves[$a_group_2['app_id']])) {
            continue;
          }

          // Group was found on one of my master/slaved apps.
          $a_result['slaves']['a_group'][$a_group_2['app_id']][$a_group_2['acl_group_id']] = $a_group_2['acl_group_id'];
        }

        // Get our app.
        $app_id = reset($mixed_app_id);

        // Get the app ids of our slaves.
        $a_app_id_slaves = array_keys($a_slaves);

        // Collect all names of the groups of our app.
        $query = mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag');
        $query->fields('ag', array(
          mediamosa_acl_group_db::ID,
          mediamosa_acl_group_db::ACL_GROUP_NAME,
          mediamosa_acl_group_db::ACL_GROUP_TYPE,
        ));
        $query->condition(mediamosa_acl_group_db::ID, $a_groups, 'IN');
        $query->condition(mediamosa_acl_group_db::APP_ID, $app_id);

        foreach ($query->execute() as $a_group) {
          // Collect all names of the groups of our app.
          $query_2 = mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag');
          $query_2->fields('ag', array(
            mediamosa_acl_group_db::APP_ID,
            mediamosa_acl_group_db::ID
          ));
          $query_2->condition(mediamosa_acl_group_db::ACL_GROUP_NAME, $a_group['acl_group_name']);
          $query_2->condition(mediamosa_acl_group_db::ACL_GROUP_TYPE, $a_group['acl_group_type']);
          $query_2->condition(mediamosa_acl_group_db::APP_ID, $app_id, '<>');

          // Now loop through all my groups that are not my app.
          foreach ($query_2->execute() as $a_group_2) {
            $query_3 = mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag');
            $query_3->fields('ag', array(
              mediamosa_acl_group_db::ID,
              mediamosa_acl_group_db::APP_ID,
            ));
            $query_3->condition(mediamosa_acl_group_db::ACL_GROUP_NAME, $a_group['acl_group_name']);
            $query_3->condition(mediamosa_acl_group_db::ACL_GROUP_TYPE, $a_group['acl_group_type']);
            $query_3->condition(mediamosa_acl_group_db::APP_ID, $a_app_id_slaves, 'IN');

            foreach ($query_3->execute() as $a_group_3) {
              $a_result['slaves_convert']['a_group'][$a_group_3['app_id']][$a_group_3['acl_group_id']] = $a_group_3['acl_group_id'];
            }
          }
        }
      }
    }

    return $a_result;
  }

  /**
   * acl_check for ownership
   *
   * Internal function to check autorisation
   * based on ownership.
   */
  public static function owner_check($app_id, $user_id, $object_app_id, $user_id_owner, $is_app_admin = FALSE) {
    assert(is_bool($is_app_admin));
    assert(!is_array($app_id));

    // we do not allow inter ega changes
    if ($app_id != $object_app_id) {
      throw new mediamosa_exception_error_access_denied(array('@reason' => 'application not allowed'));
    }

    // Ega admin of current app_id ?
    if (is_bool($is_app_admin) && $is_app_admin) {
      return;// ega admin is always owner
    }

    // basic check op owner rights : an owner has full access, the rest has none.
    if ($user_id != $user_id_owner) {
      throw new mediamosa_exception_error_access_denied(array('@reason' => 'user not allowed'));
    }
  }

  /**
   * acl_check for read access on a slaved object for viewing rights.
   * @param string $acl_type
   *  The mediamosa_acl::ACL_TYPE_*
   * @param string $object_id
   *  The ID of the object to check.
   * @param integer $app_id
   *  App ID of user.
   * @param string $user_id
   *  User to check.
   * @param integer $object_app_id
   *  App ID of the object.
   * @param string $user_id_owner
   *  User of the object.
   * @param boolean $is_app_admin
   *  TRUE or FALSE.
   */
  public static function owner_check_ext($acl_type, $object_id, $app_id, $user_id, $object_app_id, $user_id_owner, $is_app_admin = FALSE) {
    try {
      mediamosa_acl::owner_check($app_id, $user_id, $object_app_id, $user_id_owner, $is_app_admin);
    }
    catch (mediamosa_exception_error_access_denied $e) {
      // Not owner allowed or not same app (or both).
      // We still have access to the object for reading rights if we are master.
      // So check if the object is a slave of us.

      // Get the slaves of the object using the App ID of the current EGA.
      $a_slaves = mediamosa_acl_app_master_slave::slave_get($app_id, $acl_type, $object_id);

      // The app id of the object must be slave to access.
      if (!isset($a_slaves[$object_app_id])) {
        throw $e;
      }
    }
  }

  // Extension for collection, check if assign is allowed.
  /**
   * Extended owner check when creating link between asset and collection.
   *
   * You must be either owner of asset OR collection to create link.   *
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $user_id
   *  The user ID.
   * @param array $a_asset
   *  The asset.
   * @param array $a_collection
   *  The collection.
   * @param bool $is_app_admin
   *  Global switch for app_admin.
   */

  public static function owner_check_collection_assign($app_id, $user_id, array $asset, array $collection, $is_app_admin = FALSE) {
    // If the collection has public assign rights, then allow it
    if ($collection[mediamosa_collection_db::PUBLIC_ASSIGN] == mediamosa_collection_db::PUBLIC_ASSIGN_TRUE) {

      // Must be owner.
      try {
        mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);
        return; // We got access, no need to check collection ownership.
      }
      catch (mediamosa_exception_error_access_denied $e) {
        // Ignore, check ownership collection now.
        assert($e);
      }
    }

    mediamosa_acl::owner_check($app_id, $user_id, $collection[mediamosa_collection_db::APP_ID], $collection[mediamosa_collection_db::OWNER_ID], $is_app_admin);
  }

  /**
   * Test if you are app_admin
   *
   * @param integer $app_id
   * @param boolean $is_app_admin
   */
  public static function app_admin_check($app_id, $object_app_id, $is_app_admin) {
    assert(is_bool($is_app_admin));

    // We do not allow inter app changes.
    if ($app_id != $object_app_id) {
      throw new mediamosa_exception_error_access_denied(array("@reason" => "application not allowed"));
    }

    // App admin of current app_id ?
    if (!$is_app_admin) {
      throw new mediamosa_exception_error_access_denied(array("@reason" => "you must be app admin"));
    }
  }

  /**
   * Get is protected field.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $acl_type
   * @param array $a_object
   *  Make sure its an object that is within the same app_id as the user
   *  Or else it will be denied access
   * @param array $is_app_admin
   *  For future expansion
   *
   *  @return array($is_protected, $rights_set_user_usergroup, $rights_set_domain_realm)
   */
  public static function is_protected_get($app_id, $user_id, $acl_type, array $object, $is_app_admin = FALSE) {
    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['mediafile_id'];
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['asset_id'];
        break;

      case mediamosa_acl::ACL_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    $is_protected = NULL;
    $rights_set_user_usergroup = FALSE;
    $rights_set_domain_realm = FALSE;

    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        // Need to be either owner + same app or slave of object.
        mediamosa_acl::owner_check_ext($acl_type, $acl_object_id, $app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

        $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
        $query
          ->condition(mediamosa_asset_mediafile_db::ID, $acl_object_id)
          ->fields('mf', array(mediamosa_asset_mediafile_db::IS_PROTECTED));
        $result = $query->execute();
        $is_protected = $result->fetchField();

        if ($is_protected == mediamosa_asset_mediafile_db::IS_PROTECTED_USER_USERGROUP || $is_protected == mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE) {
          $rights_set_user_usergroup = TRUE;
        }
        if ($is_protected == mediamosa_asset_mediafile_db::IS_PROTECTED_DOMAIN_REALM || $is_protected == mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE) {
          $rights_set_domain_realm = TRUE;
        }

        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        // Only owners (with the same app_id) can get access (for now)
        mediamosa_acl::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

        $query = db_select(mediamosa_asset_db::TABLE_NAME, 'mf');
        $query
          ->condition(mediamosa_asset_db::ID, $acl_object_id)
          ->fields('mf', array(mediamosa_asset_db::IS_PROTECTED));
        $result = $query->execute();
        $is_protected = $result->fetchField();

        if ($is_protected == mediamosa_asset_db::IS_PROTECTED_USER_USERGROUP || $is_protected == mediamosa_asset_db::IS_PROTECTED_TRUE) {
          $rights_set_user_usergroup = TRUE;
        }
        if ($is_protected == mediamosa_asset_db::IS_PROTECTED_DOMAIN_REALM || $is_protected == mediamosa_asset_db::IS_PROTECTED_TRUE) {
          $rights_set_domain_realm = TRUE;
        }

        break;
    }

    return array($is_protected, $rights_set_user_usergroup, $rights_set_domain_realm);
  }

  /**
   * Sets rights on the object
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $acl_type
   * @param array $object
   *  Make sure its an object that is within the same app_id as the user
   *  Or else it will be denied access
   * @param array $app_ids
   *  Reserved, not supported yet.
   * @param array $acl_user_ids
   * @param array $acl_group_ids
   * @param array $acl_domains
   * @param array $acl_realms
   * @param bool $replace
   * @param array $rights
   *  For future expansion
   * @param array $is_app_admin
   */
  public static function rights_set($app_id, $user_id, $acl_type, array $object, array $app_ids, array $acl_user_ids, array $acl_group_ids, array $acl_domains, array $acl_realms, $replace = TRUE, array $rights = array(mediamosa_acl::RIGHT_ACCESS), $is_app_admin = FALSE) {
    assert(count($rights));
    assert($acl_type == mediamosa_acl::ACL_TYPE_MEDIAFILE || $acl_type == mediamosa_acl::ACL_TYPE_ASSET || $acl_type == mediamosa_acl::ACL_TYPE_COLLECTION); // media files for now

    assert(!is_null($app_ids));
    assert(!is_null($acl_user_ids));
    assert(!is_null($acl_group_ids));
    assert(!is_null($acl_domains));
    assert(!is_null($acl_realms));

    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['mediafile_id'];
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['asset_id'];
        break;

      case mediamosa_acl::ACL_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Only owners (with the same app_id) can change access (for now)
    mediamosa_acl::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

    $rights_set = $rights_set_user_usergroup = $rights_set_domain_realm = FALSE;

    // Replace old existing rights.
    if ($replace) {
      mediamosa_acl::rights_clear($app_id, $user_id, $acl_type, $object, $is_app_admin);
    }
    else {
      // Get ACL rights, evaluate rights->isprotected.
      list(, $rights_set_user_usergroup, $rights_set_domain_realm) = mediamosa_acl::is_protected_get($app_id, $user_id, $acl_type, $object, $is_app_admin);
    }


    $result = array();

    // Link app access.
    // Master / Slave;
    // Only the owner can share his media to another app
    // The other app is then called 'master', and the media shared is the slave.
    foreach ($app_ids as $app_id_master) {

      try {
        mediamosa_acl_app_master_slave::create($acl_object_id, $acl_type, $app_id_master, $app_id);

        $result[] = array(
          'type' => mediamosa_acl::ACL_NAME_TYPE_APP,
          'value' => $app_id_master,
          'a_error' => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          'type' => mediamosa_acl::ACL_NAME_TYPE_APP,
          'value' => $app_id_master,
          'a_error' => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    // Link users access
    foreach ($acl_user_ids as $acl_user_id) {
      try {
        $acl_id = mediamosa_acl_name::create($app_id, $acl_user_id, mediamosa_acl::ACL_NAME_TYPE_USER);
        if (!$acl_id) {
          throw new mediamosa_exception_error_unexpected_error();
        }

        // Now link acl_name with object (using acl_object)
        mediamosa_acl_object::link_to_object($acl_object_id, mediamosa_acl_object_db::ACL_TYPE_NAME, $acl_type, $acl_id);

        $rights_set = $rights_set_user_usergroup = TRUE;
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_USER,
          "value" => $acl_user_id,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_USER,
          "value" => $acl_user_id,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    foreach ($acl_group_ids as $acl_group_id) {
      try {
        $acl_id = mediamosa_acl_name::create($app_id, $acl_group_id, mediamosa_acl::ACL_NAME_TYPE_USER_GROUP);
        assert($acl_id);
        if (!$acl_id) {
          throw new mediamosa_exception_error_unexpected_error();
        }

        // Now link acl_name with object (using acl_object)
        mediamosa_acl_object::link_to_object($acl_object_id, mediamosa_acl_object_db::ACL_TYPE_NAME, $acl_type, $acl_id);

        $rights_set = $rights_set_user_usergroup = TRUE;
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_USER_GROUP,
          "value" => $acl_group_id,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_USER_GROUP,
          "value" => $acl_group_id,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    foreach ($acl_domains as $acl_domain) {
      try {
        // Check if domain is group
        if (mediamosa_acl::is_group_domain($acl_domain)) { //[...].group domain
          $acl_id = mediamosa_acl_group::create($app_id, $acl_domain, mediamosa_acl::ACL_NAME_TYPE_DOMAIN);
          if (!$acl_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link acl_name with object (using acl_object)
          mediamosa_acl_object::link_to_object($acl_object_id, mediamosa_acl_object_db::ACL_TYPE_GROUP, $acl_type, $acl_id);
        }
        else {
          $acl_id = mediamosa_acl_name::create($app_id, $acl_domain, mediamosa_acl::ACL_NAME_TYPE_DOMAIN);
          if (!$acl_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link acl_name with object (using acl_object)
          mediamosa_acl_object::link_to_object($acl_object_id, mediamosa_acl_object_db::ACL_TYPE_NAME, $acl_type, $acl_id);
        }

        $rights_set = $rights_set_domain_realm = TRUE;
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_DOMAIN,
          "value" => $acl_domain,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_DOMAIN,
          "value" => $acl_domain,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    foreach ($acl_realms as $acl_realm) {
      try {
        // They can also define groups here...
        if (mediamosa_acl::is_group_realm($acl_realm)) { //@[...].group realm
          $acl_id = mediamosa_acl_group::create($app_id, $acl_realm, mediamosa_acl::ACL_NAME_TYPE_REALM);
          assert($acl_id);
          if (!$acl_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link acl_name with object (using acl_object)
          mediamosa_acl_object::link_to_object($acl_object_id, mediamosa_acl_object_db::ACL_TYPE_GROUP, $acl_type, $acl_id);
        }
        else {
          $acl_id = mediamosa_acl_name::create($app_id, $acl_realm, mediamosa_acl::ACL_NAME_TYPE_REALM);
          if (!$acl_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link acl_name with object (using acl_object)
          mediamosa_acl_object::link_to_object($acl_object_id, mediamosa_acl_object_db::ACL_TYPE_NAME, $acl_type, $acl_id);
        }

        $rights_set = $rights_set_domain_realm = TRUE;
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_REALM,
          "value" => $acl_realm,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_acl::ACL_NAME_TYPE_REALM,
          "value" => $acl_realm,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    if ($rights_set) {
      switch ($acl_type) {
        case mediamosa_acl::ACL_TYPE_MEDIAFILE:
          if (!$rights_set_domain_realm && $rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_acl::MEDIAFILE_IS_PROTECTED_USER_USERGROUP,
            );
          }
          elseif ($rights_set_domain_realm && !$rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM,
            );
          }
          else {
            $fields = array(
              mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE,
            );
          }

          // Add changed value.
          $fields = mediamosa_db::db_update_enrich($fields);

          // Update it.
          mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
            ->fields($fields)
            ->condition(mediamosa_asset_mediafile_db::ID, $acl_object_id)
            ->execute();

          // ACL status changed, remove possible public link.
          mediamosa_media::remove_public_link($app_id, $acl_object_id);

          // Reindex the asset using the mediafile ID.
          mediamosa_asset_mediafile::mediamosa_asset_reindex(array($acl_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_ACL);
          break;

        case mediamosa_acl::ACL_TYPE_ASSET:
          if (!$rights_set_domain_realm && $rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_db::IS_PROTECTED => mediamosa_acl::MEDIAFILE_IS_PROTECTED_USER_USERGROUP,
            );
          }
          elseif ($rights_set_domain_realm && !$rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_db::IS_PROTECTED => mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM,
            );
          }
          else {
            $fields = array(
              mediamosa_asset_db::IS_PROTECTED => mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE,
            );
          }

          // Add changed value.
          $fields = mediamosa_db::db_update_enrich($fields);

          // Update it.
          mediamosa_db::db_update(mediamosa_asset_db::TABLE_NAME)
            ->fields($fields)
            ->condition(mediamosa_asset_db::ID, $acl_object_id)
            ->execute();

          // ACL status changed, remove possible perm. link.
          mediamosa_media::remove_public_link_asset($acl_object_id);

          // Reindex the asset.
          mediamosa_asset::mediamosa_asset_reindex(array($acl_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_ACL);
          break;

        case mediamosa_acl::ACL_TYPE_COLLECTION:
          break; // nothing yet

        default:
          throw new mediamosa_exception_error_unexpected_error();
      }
    }

    return $result;
  }

  /**
   * Remove rights from object.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The owner of the object.
   * @param string $acl_type
   *   The type of object, mediafile, asset or collection. See
   *   mediamosa_acl::ACL_TYPE_*.
   * @param array $object
   *   An assocative array with owner, app and mediafile IDs data.
   * @param boolean $is_app_admin
   *   Overrulling of owner check.
   */
  public static function rights_clear($app_id, $user_id, $acl_type, array $object, $is_app_admin = FALSE) {
    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['mediafile_id'];
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['asset_id'];
        break;

      case mediamosa_acl::ACL_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Only owners (from the same app_id) can change access.
    mediamosa_acl::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $fields = array(
          mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE,
        );

        // Add changed value.
        $fields = mediamosa_db::db_update_enrich($fields);

        // Update it.
        mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
          ->fields($fields)
          ->condition(mediamosa_asset_mediafile_db::ID, $acl_object_id)
          ->execute();
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        $fields = array(
          mediamosa_asset_db::IS_PROTECTED => mediamosa_asset_db::IS_PROTECTED_FALSE,
        );

        // Add changed value.
        $fields = mediamosa_db::db_update_enrich($fields);

        // Update it.
        mediamosa_db::db_update(mediamosa_asset_db::TABLE_NAME)
          ->fields($fields)
          ->condition(mediamosa_asset_db::ID, $acl_object_id)
          ->execute();
        break;

      case mediamosa_acl::ACL_TYPE_COLLECTION:
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Remove all links between object and the acl_name/acl_group tabel.
    // This will result into removal of all rights.
    // Also remove the master / slave links
    mediamosa_db::db_query('DELETE FROM {mediamosa_acl_object} WHERE acl_object_type = :acl_object_type AND acl_object_id = :acl_object_id', array(
      ':acl_object_type' => $acl_type,
      ':acl_object_id' => $acl_object_id,
    ));

    // Remove the master / slave link, only the original app_id kan remove
    // these.
    mediamosa_db::db_query('DELETE FROM {mediamosa_acl_app_master_slave} WHERE acl_object_type = :acl_object_type AND acl_object_id = :acl_object_id AND app_id_master > 0 AND app_id_slave = :app_id_slave', array(
      ':acl_object_type' => $acl_type,
      ':acl_object_id' => $acl_object_id,
      ':app_id_slave' => $app_id,
    ));

    // Reindex object.
    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        // ACL status changed, remove possible perm. link.
        mediamosa_media::remove_public_link($object_app_id, $acl_object_id);

        // Reindex the asset using the mediafile ID.
        mediamosa_asset_mediafile::mediamosa_asset_reindex(array($acl_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_ACL);
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        // ACL status changed, remove possible perm. link.
        mediamosa_media::remove_public_link_asset($acl_object_id);

        // Reindex the asset using the mediafile ID.
        mediamosa_asset::mediamosa_asset_reindex(array($acl_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_ACL);
        break;
    }
  }

  /**
   * Get the ACL rights of object.
   * Will check ownership of object.
   * On media it will allow slaves to get rights of object when not owner.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $acl_type
   * @param array $object
   * @param boolean $is_app_admin
   * @return array
   */
  public static function rights_get($app_id, $user_id, $acl_type, $object, $is_app_admin = FALSE) {
    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['mediafile_id'];
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['asset_id'];
        break;

      case mediamosa_acl::ACL_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $acl_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        // Need to be either owner + same app or slave of object.
        mediamosa_acl::owner_check_ext($acl_type, $acl_object_id, $app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);
        break;

      default:
        // Only owners (with the same app_id) can get access (for now)
        mediamosa_acl::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);
        break;
    }

    // Haal eerst de acl_id, zo we weten welke links er zijn
    $db_result = mediamosa_db::db_query(
      'SELECT #acl_id, #acl_type FROM {#mediamosa_acl_object} WHERE #acl_object_type = :acl_object_type AND #acl_object_id = :acl_object_id',
      array(
        '#mediamosa_acl_object' => mediamosa_acl_object_db::TABLE_NAME,
        '#acl_id' => mediamosa_acl_object_db::ACL_ID,
        '#acl_type' => mediamosa_acl_object_db::ACL_TYPE,
        '#acl_object_type' => mediamosa_acl_object_db::ACL_OBJECT_TYPE,
        ':acl_object_type' => $acl_type,
        '#acl_object_id' => mediamosa_acl_object_db::ACL_OBJECT_ID,
        ':acl_object_id' => $acl_object_id,
      )
    );

    $a_acl_ids = array();
    foreach ($db_result as $a_row) {
      $a_acl_ids[$a_row['acl_type']][] = $a_row['acl_id'];
    }

    $a_result = array(mediamosa_acl_object_db::ACL_TYPE_NAME => array(), mediamosa_acl_object_db::ACL_TYPE_GROUP => array(), mediamosa_acl::ACL_NAME_TYPE_APP => array());

    $db_result_name = $db_result_group = FALSE;

    if (isset($a_acl_ids[mediamosa_acl_object_db::ACL_TYPE_NAME]) && !empty($a_acl_ids[mediamosa_acl_object_db::ACL_TYPE_NAME])) {
      $db_result_name = mediamosa_db::db_select(mediamosa_acl_name_db::TABLE_NAME, 'an')
        ->fields('an')
        ->condition(mediamosa_acl_name_db::ID, $a_acl_ids[mediamosa_acl_object_db::ACL_TYPE_NAME], 'IN')
        ->execute();
    }

    if (isset($a_acl_ids[mediamosa_acl_object_db::ACL_TYPE_GROUP]) && !empty($a_acl_ids[mediamosa_acl_object_db::ACL_TYPE_GROUP])) {
      $db_result_group = mediamosa_db::db_select(mediamosa_acl_group_db::TABLE_NAME, 'ag')
        ->fields('ag')
        ->condition(mediamosa_acl_group_db::ID, $a_acl_ids[mediamosa_acl_object_db::ACL_TYPE_GROUP], 'IN')
        ->execute();
    }

    // Master / slave(s)
    // Note / todo ;
    // does the owner of the mediafile have the right to see other slave rights from other master app (not his app)?
    // For now we only allow rights that owner might have set (he cant set or remove app slave rights if he/she does not have the same master app
    $db_result_master_slave = mediamosa_db::db_query("SELECT * FROM {mediamosa_acl_app_master_slave} WHERE acl_object_type = :acl_object_type AND acl_object_id = :acl_object_id AND app_id_master > 0 AND app_id_slave = :app_id_slave", array(
      ':acl_object_type' => $acl_type,
      ':acl_object_id' => $acl_object_id,
      ':app_id_slave' => $object_app_id,
    ));

    if ($db_result_name) {
      foreach ($db_result_name as $a_row) {
        $name = $a_row['acl_name'];
        if ($a_row['acl_type'] == mediamosa_acl::ACL_NAME_TYPE_REALM)  {
          $name = $a_row['acl_prefix'] . $a_row['acl_name'];
        }

        $a_result[mediamosa_acl_object_db::ACL_TYPE_NAME][] = array('type' => 'acl_' . mediamosa_unicode::strtolower($a_row['acl_type']), 'name' => $name);
      }
    }

    if ($db_result_group) {
      foreach ($db_result_group as $a_row) {
        $a_result[mediamosa_acl_object_db::ACL_TYPE_GROUP][] = array('type' => 'acl_' . mediamosa_unicode::strtolower($a_row['acl_group_type']), 'name' => $a_row['acl_group_name']);
      }
    }

    // Master / slave
    // Master is de share, zodat master onze object mag zien
    foreach ($db_result_master_slave as $a_master_slave) {
      $a_result[mediamosa_acl::ACL_NAME_TYPE_APP][] = array('type' => 'acl_app', 'app_id_slave' => $a_master_slave['app_id_slave'], 'app_id_master' => $a_master_slave['app_id_master']);
    }

    return $a_result;
  }

  /**
   * Copy the ACL rights, slave/master from one object to another, does not have to be the same object type
   * Remember that will add right(s), does not remove current rights.
   *
   * @param string $acl_type_source
   * @param string $acl_id_source
   * @param string $acl_type_dest
   * @param string $acl_id_dest
   */
  public static function replace_obj_to_obj($acl_type_source, $acl_id_source, $acl_type_dest, $acl_id_dest) {

    // We replace, so remove any existing rights 1st.
    mediamosa_db::db_query("DELETE FROM {mediamosa_acl_object} WHERE acl_object_type=:acl_object_type AND acl_object_id=:acl_object_id", array(
      ':acl_object_type' => $acl_type_dest,
      ':acl_object_id' => $acl_id_dest,
    ));
    mediamosa_db::db_query("DELETE FROM {mediamosa_acl_app_master_slave} WHERE acl_object_type=:acl_object_type AND acl_object_id=:acl_object_id", array(
      ':acl_object_type' => $acl_type_dest,
      ':acl_object_id' => $acl_id_dest,
    ));

    // Copy normal ACL rights
    $db_result = mediamosa_db::db_query("SELECT acl_type, acl_id FROM {mediamosa_acl_object} WHERE acl_object_type = :acl_object_type AND acl_object_id = :acl_object_id", array(
      ':acl_object_type' => $acl_type_source,
      ':acl_object_id' => $acl_id_source,
    ));
    foreach ($db_result as $dbrow) {
      mediamosa_db::db_query("INSERT INTO {mediamosa_acl_object} SET acl_object_id = :acl_object_id, acl_type = :acl_type, acl_id = :acl_id, acl_object_type = :acl_object_type", array(
        ':acl_object_id' => $acl_id_dest,
        ':acl_type' => $dbrow['acl_type'],
        ':acl_id' => $dbrow['acl_id'],
        ':acl_object_type' => $acl_type_dest,
      ));
    }

    // Copy master/slave settings
    $db_result = mediamosa_db::db_query("SELECT app_id_slave, app_id_master FROM {mediamosa_acl_app_master_slave} WHERE acl_object_type = :acl_object_type AND acl_object_id = :acl_object_id", array(
      ':acl_object_type' => $acl_type_source,
      ':acl_object_id' => $acl_id_source,
    ));
    foreach ($db_result as $dbrow) {
      mediamosa_db::db_query("INSERT INTO {mediamosa_acl_app_master_slave} SET acl_object_id = :acl_object_id, acl_object_type = :acl_object_type, app_id_slave = :app_id_slave, app_id_master=:app_id_master", array(
        ':acl_object_id' => $acl_id_dest,
        ':acl_object_type' => $acl_type_dest,
        ':app_id_slave' => $dbrow['app_id_slave'],
        ':app_id_master' => $dbrow['app_id_master'],
      ));
    }

    // Now copy the is_protected from source to dest.
    switch ($acl_type_source) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $is_protected = mediamosa_db::db_query("SELECT is_protected FROM {mediamosa_asset_mediafile} WHERE mediafile_id = :mediafile_id", array(':mediafile_id' => $acl_id_source))->fetchAssoc();
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        $is_protected = mediamosa_db::db_query("SELECT is_protected FROM {mediamosa_asset} WHERE asset_id = :asset_id", array(':asset_id' => $acl_id_source))->fetchAssoc();
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Now update the new object with the copy is_protected.
    switch ($acl_type_source) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        mediamosa_db::db_query("UPDATE {mediamosa_asset_mediafile} SET is_protected = :is_protected WHERE mediafile_id = :mediafile_id", array(
          ':is_protected' => $is_protected,
          ':mediafile_id' => $acl_id_dest,
        ));

        // Reindex the asset using the mediafile ID.
        mediamosa_asset_mediafile::mediamosa_asset_reindex(array($acl_id_dest), mediamosa_settings::SEARCH_INDEX_TYPE_ACL);
        break;

      case mediamosa_acl::ACL_TYPE_ASSET:
        mediamosa_db::db_query("UPDATE {mediamosa_asset} SET is_protected = :is_protected WHERE asset_id = :asset_id", array(
          ':is_protected' => $is_protected,
          ':asset_id' => $acl_id_dest,
        ));

        // Reindex the asset using the mediafile ID.
        mediamosa_asset::mediamosa_asset_reindex(array($acl_id_dest), mediamosa_settings::SEARCH_INDEX_TYPE_ACL);
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }
  }

  /**
   * Copy the ACL rights from one mediafile to another mediafile
   *
   * @param string $mediafile_id_src
   * @param string $mediafile_id_dest
   */
  public static function replace_mediafile_to_mediafile($mediafile_id_src, $mediafile_id_dest) {
    return mediamosa_acl::replace_obj_to_obj(mediamosa_acl::ACL_TYPE_MEDIAFILE, $mediafile_id_src, mediamosa_acl::ACL_TYPE_MEDIAFILE, $mediafile_id_dest);
  }

  /**
   * Creates the sql to include in your sql call on object to test access
   *
   * @param array $query
   * @param string $acl_type
   *  see ACL_TYPE_*
   * @param string $object_id
   *  If given, we check only on this object for rights.
   *  This is also the main switch between a search query and an object search
   *  Specify NULL for non-direct matching.
   * @param array $app_ids
   * @param string $acl_user_id
   * @param array $acl_group_ids
   * @param string $acl_domain
   * @param string $acl_realm
   * @param array $slaves
   * @param boolean $is_app_admin
   * @param boolean $do_master_slave_only
   * @param boolean $do_master_slave_null
   * @param string $app_id_table_prefix
   */
  public static function build_access_where(array &$query, $acl_type, $object_id, array $app_ids, $acl_user_id, $acl_group_ids = array(), $acl_domain, $acl_realm, array $slaves = array(), $is_app_admin = FALSE, $do_master_slave_only = FALSE, $do_master_slave_null = FALSE, $app_id_table_prefix = 'a') {
    assert(is_bool($is_app_admin));

    if (empty($slaves)) {
      $slaves = mediamosa_acl_app_master_slave::slave_get($app_ids, $acl_type);
    }

    // Also require the rights from the other possible slave app(s)
    $app_ids_tmp = array_unique(array_merge($app_ids, array_keys($slaves)));

    // Get of this current user the acl_name/acl_group ids so we know his/her access.
    $a_acl_ids = mediamosa_acl::build_access($app_ids_tmp, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, $slaves);

    if (!$do_master_slave_only && $app_id_table_prefix == 'a' && ($acl_type == mediamosa_acl::ACL_TYPE_ASSET || $acl_type == mediamosa_acl::ACL_TYPE_MEDIAFILE)) {
      $now = mediamosa_datetime::utc_current_timestamp_now();
      $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['play_restriction'][] = strtr('(a.play_restriction_start IS NULL OR a.play_restriction_start < :now)', array(':now' => mediamosa_db::escape_string_quoted($now)));
      $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['play_restriction'][] = strtr('(a.play_restriction_end IS NULL OR a.play_restriction_end > :now)', array(':now' => mediamosa_db::escape_string_quoted($now)));
    }

    if (!is_null($acl_user_id) || (!is_null($acl_group_ids) && !empty($acl_group_ids)) || count($a_acl_ids) || count($slaves)) {
      switch ($acl_type) {
        case mediamosa_acl::ACL_TYPE_MEDIAFILE:

          $query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id = mf.asset_id";
          // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps
          if (!$do_master_slave_only) {
            // If $do_master_slave_only is TRUE, granted is TRUE, meaning we need to return all assets
            // even if we dont have access. This switch prevents returning all assets of the other
            // slave apps.

            // If we need to check acl_object
            if (count($a_acl_ids)) {
              $query[mediamosa_db_query::A_JOIN]['acl_object'] = mediamosa_acl_object::join_acl_object_get($acl_type);
            }

            // allow assets without mediafile for master app_id
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = sprintf('a.app_id IN(%s)', implode(',', $app_ids));
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = 'mf.asset_id IS NULL';

            if (is_null($object_id)) {
              // Removed according to ticket #432.
              //$query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file='TRUE'";
              // Added according to ticket #521.
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.is_still='FALSE'";
            }
            else {
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.mediafile_id='" . mediamosa_db::escape_string($object_id) . "'";
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = 'mf.is_original_file IS NOT NULL';// always TRUE, because its never NULL
            }

            // If we have acl id for the master app, check them
            foreach ($app_ids as $app_id) {
              if (!empty($a_acl_ids[$app_id])) {
                // Master app_id checks
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "mf.app_id=" . mediamosa_db::escape_string($app_id);

                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR][] = "mf.is_protected = 'FALSE'";

                if (count($a_acl_ids[$app_id]['a_name'])) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_type = 'NAME'";
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_id IN(" . implode(",", $a_acl_ids[$app_id]['a_name']) . ")";
                }

                if (count($a_acl_ids[$app_id]['a_group'])/* || (isset($a_acl_ids['slaves']) && isset($a_acl_ids['slaves']['a_group'][$app_id]))*/) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_type = 'GROUP'";
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_id IN(" . implode(",", $a_acl_ids[$app_id]['a_group']) . ")";
                }
              }
              else {
                // allow master app_id media when not protected only
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "mf.app_id=" . mediamosa_db::escape_string($app_id);
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR][] = "mf.is_protected = 'FALSE'";
              }

              if (!is_null($acl_user_id)) {
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]['user_id'] = "mf.owner_id = '" . mediamosa_db::escape_string($acl_user_id) . "'";
              }
              // group_id
              if (!is_null($acl_group_ids)) {
                assert(is_array($acl_group_ids));

                foreach ($acl_group_ids as $acl_group_id) {
                  if (!empty($acl_group_id)) {
                    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]['group_id'][] = "mf.group_id = '" . mediamosa_db::escape_string($acl_group_id) . "'";
                  }
                }
              }
            }
          }
          else {
            // When $do_master_slave_only is TRUE, we have always access to media of our own app_id(s)

            // Master app_id checks
            // Because the $app_ids is in the app_id selection of assets, this one is a bit useles...
            if (count($app_ids)) {
              // Master app_id checks
              if ($do_master_slave_null) {
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR]['master_app_id'][mediamosa_db_query::WHERE_AND]['ms'] = strtr('@app_id_table_prefix.app_id IN(@app_ids) OR @app_id_table_prefix.app_id IS NULL', array('@app_id_table_prefix' => $app_id_table_prefix, '@app_ids' => implode(',', $app_ids)));
              }
              else {
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR]['master_app_id'][mediamosa_db_query::WHERE_AND]['ms'] = strtr('@app_id_table_prefix.app_id IN(@app_ids)', array('@app_id_table_prefix' => $app_id_table_prefix, '@app_ids' => implode(',', $app_ids)));
              }
            }
          }

          // possible slaves
          if (count($slaves)) {
            $query[mediamosa_db_query::A_JOIN]['acl_app_master_slave'] = mediamosa_acl_app_master_slave::join_acl_app_master_slave_get($acl_type);

            // If we need to check acl_object
            if (count($a_acl_ids)) {
              $query[mediamosa_db_query::A_JOIN]['acl_object'] = mediamosa_acl_object::join_acl_object_get($acl_type);
            }

            foreach ($slaves as $app_id_slave => $a_slave) {
              assert($app_id_slave);// should not happen
              // Only for app_ids that are present

              // Must be same app_id for slaves
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = $app_id_table_prefix . '.app_id=' . mediamosa_db::escape_string($app_id_slave);

              // Any case, a link to app must be present if we allow access to the MF
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("acl_ms.app_id_master IN(%s)", implode(",", $app_ids));
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("acl_ms.app_id_slave = %d", $app_id_slave);

              if ($do_master_slave_only) {
                // If we only do master/slave, we include all that has been slaved and protected
                continue;
              }

              // 2 cases;
              // 1. The object is protected only app_id (is_protected == FALSE)
              //   then only access if app_id == slave_app_id
              // 2. The object is protected by other rights (is_protected == TRUE)
              //   then only access if app_id == slave_app_id and rights match with other rights

              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

              if (isset($a_acl_ids[$app_id_slave]) && count($a_acl_ids[$app_id_slave])) {
                if (count($a_acl_ids[$app_id_slave]['a_name'])) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
                }

                // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
                if (count($a_acl_ids[$app_id_slave]['a_name'])) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_type='NAME'";
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_id IN(" . implode(",", $a_acl_ids[$app_id_slave]['a_name']) . ")";
                }
              }

              if (isset($a_acl_ids['slaves_convert']) && isset($a_acl_ids['slaves_convert']['a_group'][$app_id_slave])) {
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_type='GROUP'";
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obj.acl_id IN(" . implode(",", $a_acl_ids['slaves_convert']['a_group'][$app_id_slave]) . ")";
              }
            }
          }

          break;

        case mediamosa_acl::ACL_TYPE_ASSET:

          // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps
          if (!$do_master_slave_only) {
            // If $do_master_slave_only is TRUE, granted is TRUE, meaning we need to return all assets
            // even if we dont have access. This switch prevents returning all assets of the other
            // slave apps.

            // If we need to check acl_object
            if (count($a_acl_ids)) {
              $query[mediamosa_db_query::A_JOIN]['acl_object_asset'] = mediamosa_acl_object::join_acl_object_get($acl_type);
            }
/*
            // allow assets without mediafile for master app_id
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = sprintf('a.app_id IN(%s)', implode(',', $app_ids));
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = 'mf.asset_id IS NULL';
/*
 *
 * Hmmm hmmm
            if (is_null($object_id)) {
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file='TRUE'";
            }
            else {
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.mediafile_id='" . mediamosa_db::escape_string($object_id) . "'";
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = 'mf.is_original_file IS NOT NULL';// always TRUE, because its never NULL
            }
 */
            // If we have acl id for the master app, check them
            foreach ($app_ids as $app_id) {
              if (!empty($a_acl_ids[$app_id])) {
                // Master app_id checks
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "a.app_id=" . mediamosa_db::escape_string($app_id);

                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR][] = "a.is_protected = 'FALSE'";
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = "a.is_protected != 'FALSE'";

                if (!empty($a_acl_ids[$app_id]['a_name'])) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_type = 'NAME'";
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_id IN(" . implode(",", $a_acl_ids[$app_id]['a_name']) . ")";
                }

                if (!empty($a_acl_ids[$app_id]['a_group']) || !empty($a_acl_ids['slaves']['a_group'][$app_id])) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_type = 'GROUP'";
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_id IN(" . implode(",", $a_acl_ids[$app_id]['a_group']) . ")";
                }
              }
              else {
                // allow master app_id media when not protected only
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "a.app_id=" . mediamosa_db::escape_string($app_id);
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR][] = "a.is_protected = 'FALSE'";
              }

              if (!is_null($acl_user_id)) {
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]['user_id'] = "a.owner_id = '" . mediamosa_db::escape_string($acl_user_id) . "'";
              }

              // group_id
              if (!is_null($acl_group_ids)) {
                assert(is_array($acl_group_ids));

                foreach ($acl_group_ids as $acl_group_id) {
                  if (!empty($acl_group_id)) {
                    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]['group_id'][] = "a.group_id = '" . mediamosa_db::escape_string($acl_group_id) . "'";
                  }
                }
              }
            }
          }
          else {
            // When $do_master_slave_only is TRUE, we have always access to media of our own app_id(s)

            // Master app_id checks
            // Because the $app_ids is in the app_id selection of assets, this one is a bit useles...
            if (count($app_ids)) {
            // Master app_id checks
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR]['master_app_id'][mediamosa_db_query::WHERE_AND]['ms'] = sprintf('a.app_id IN(%s)', implode(',' , $app_ids));
            }
          }

          // possible slaves
          if (count($slaves)) {
            $query[mediamosa_db_query::A_JOIN]['acl_app_master_slave_asset'] = mediamosa_acl_app_master_slave::join_acl_app_master_slave_get($acl_type);

            // If we need to check acl_object
            if (count($a_acl_ids)) {
              $query[mediamosa_db_query::A_JOIN]['acl_object_asset'] = mediamosa_acl_object::join_acl_object_get($acl_type);
            }

            foreach ($slaves as $app_id_slave => $a_slave) {
              assert($app_id_slave);// should not happen
              // Only for app_ids that are present

              // Must be same app_id for slaves
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = "a.app_id ='" . mediamosa_db::escape_string($app_id_slave);

              // Any case, a link to app must be present if we allow access to the MF
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("acl_msa.app_id_master IN(%s)", implode(",", $app_ids));
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("acl_msa.app_id_slave = %d", $app_id_slave);

              if ($do_master_slave_only) {
                // If we only do master/slave, we include all that has been slaved and protected
                continue;
              }

              // 2 cases;
              // 1. The object is protected only app_id (is_protected == FALSE)
              //   then only access if app_id == slave_app_id
              // 2. The object is protected by other rights (is_protected == TRUE)
              //   then only access if app_id == slave_app_id and rights match with other rights

              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

              if (isset($a_acl_ids[$app_id_slave]) && count($a_acl_ids[$app_id_slave])) {
                // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
                if (count($a_acl_ids[$app_id_slave]['a_name'])) {
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_type='NAME'";
                  $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_id IN(" . implode(",", $a_acl_ids[$app_id_slave]['a_name']) . ")";
                }
              }

              if (isset($a_acl_ids['slaves_convert']) && isset($a_acl_ids['slaves_convert']['a_group'][$app_id_slave])) {
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_type='GROUP'";
                $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]['is_protected'][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "acl_obja.acl_id IN(" . implode(",", $a_acl_ids['slaves_convert']['a_group'][$app_id_slave]) . ")";
              }
            }
          }
          break;

        case mediamosa_acl::ACL_TYPE_COLLECTION:
          // Allow master app always
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][0][mediamosa_db_query::WHERE_AND][] = sprintf('c.app_id IN(%s)', implode(',', $app_ids));

          if (count($slaves)) {
            $query[mediamosa_db_query::A_JOIN]['acl_app_master_slave_coll'] = mediamosa_acl_app_master_slave::join_acl_app_master_slave_get($acl_type);

            foreach ($slaves as $app_id_slave => $a_slave) {
              // Must be same app_id for slaves
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = 'c.app_id = ' . mediamosa_db::escape_string($app_id_slave);

              // Any case, a link to app must be present if we allow access to the MF
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf('acl_msc.app_id_master IN(%s)', implode(',', $app_ids));
              $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf('acl_msc.app_id_slave = %d', $app_id_slave);
            }
          }
          break;

        default:
          assert(0);
          return;
      }
    }
    else {
      switch ($acl_type) {
        case mediamosa_acl::ACL_TYPE_MEDIAFILE:
          // Include the mediafile table for join
          $query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id = mf.asset_id";

          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['no_mf'] = "mf.asset_id IS NULL";
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id'][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file = 'TRUE'";
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id'][mediamosa_db_query::WHERE_AND][] = sprintf("mf.app_id IN(%s)", implode(',', $app_ids));

          if (!$do_master_slave_only) {
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id'][mediamosa_db_query::WHERE_AND][] = "mf.is_protected = 'FALSE'";
          }
          break;

        case mediamosa_acl::ACL_TYPE_ASSET:
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id_asset'][mediamosa_db_query::WHERE_AND][] = sprintf('a.app_id IN(%s)', implode(',', $app_ids));

          if (!$do_master_slave_only) {
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id_asset'][mediamosa_db_query::WHERE_AND][] = "a.is_protected = 'FALSE'";
          }
          break;

        case mediamosa_acl::ACL_TYPE_COLLECTION:
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection'][] = sprintf('c.app_id IN(%s)', implode(',', $app_ids));

          break;

        default:
          assert(0);
          return;
      }
    }
  }

  /**
   * Check access on assets using given array asset_ids
   *
   * @param array $a_asset_ids
   * @param integer $app_id
   * @param string $acl_user_id
   * @param array $acl_group_ids
   * @param string $s_acl_domain
   * @param string $s_acl_realm
   * @return array
   *  returns the array with asset_id(s) on which we have access on
   */
  public static function access_check_assets($asset_ids, array $app_ids, $acl_user_id, array $acl_group_ids, $acl_domain, $acl_realm, $is_app_admin = FALSE) {

    $query = array();

    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.asset_id';
    $query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset} AS a';

    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id IN('%s')", implode("','", $asset_ids));

    // Build the access where.
    mediamosa_acl::build_access_where($query, mediamosa_acl::ACL_TYPE_MEDIAFILE, NULL, $app_ids, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, array(), $is_app_admin);

    $query_sql = mediamosa_db_query::query_select($query);

    $db_result = mediamosa_db::db_query($query_sql);

    $asset_ids = array();
    foreach ($db_result as $row) {
      $asset_ids[$row['asset_id']] = $row['asset_id'];
    }

    return $asset_ids;
  }

  /**
   * Check access on a single object
   *
   * @param string $acl_type
   * @param string $asset_id
   * @param string $object_id
   * @param integer $app_id
   * @param string $acl_user_id
   * @param array $acl_group_ids
   * @param string $s_acl_domain
   * @param string $s_acl_realm
   *
   * @throws mediamosa_exception_error_access_denied (ERRORCODE_NOT_AUTHORIZED) when access is denied
   */
  public static function access_check_on_object($acl_type, $asset_id, $object_id, $app_id, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, $is_app_admin = FALSE) {

    if ($is_app_admin) {
      return;
    }

    assert(!is_null($object_id));
    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:

        $query = array();

        $query[mediamosa_db_query::A_SELECT_EXPR][] = "a.asset_id";
        $query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

        // Build the access where
        mediamosa_acl::build_access_where($query, $acl_type, $object_id, array($app_id), $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, array(), $is_app_admin);

        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id = '%s'", $asset_id);
        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

        if (!$is_app_admin) {
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = "a.is_unappropriate = 'FALSE'";

          if ($acl_user_id) {// if provided, then we must be owner to access unappropiate assets
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = sprintf("(a.is_unappropriate = 'TRUE' AND a.owner_id='%s')", mediamosa_db::escape_string($acl_user_id));
          }
        }

        // Convert into SQL.
        $query_sql = mediamosa_db_query::query_select($query);

        // Now execute the SQL.
        $db_result = mediamosa_db::db_query($query_sql);

        if ($db_result->fetchAssoc() === FALSE) {
          // FIXME: will always be thrown, even when unappropriate was the reason.
          throw new mediamosa_exception_error_access_denied(array('@reason' => 'mediafile is protected'));
        }

        break;

      case mediamosa_acl::ACL_TYPE_ASSET:

        $query = array();

        $query[mediamosa_db_query::A_SELECT_EXPR][] = "a.asset_id";
        $query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

        // Build the access where
        mediamosa_acl::build_access_where($query, $acl_type, $object_id, array($app_id), $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, array(), $is_app_admin);

        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id ='%s'", $asset_id);
        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

        if (!$is_app_admin) {
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = "a.is_unappropriate = 'FALSE'";

          if ($acl_user_id) {// if provided, then we must be owner to access unappropiate assets
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = sprintf("(a.is_unappropriate = 'TRUE' AND a.owner_id = '%s')", mediamosa_db::escape_string($acl_user_id));
          }
        }

        // Convert into SQL.
        $query_sql = mediamosa_db_query::query_select($query);

        // Now execute the SQL.
        $db_result = mediamosa_db::db_query($query_sql);

        if ($db_result->fetchAssoc() === FALSE) {
          throw new mediamosa_exception_error_access_denied(array('@reason' => 'asset is protected'));
        }

        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }
  }

  /**
   * Check if the object has a valid slave record when it's a foreign object
   */
  public static function read_single_object($type, $id, array $app_ids) {

    switch ($type) {
      case mediamosa_acl::ACL_TYPE_ASSET:

        // Basic app_id check
        $asset_id_app_id = mediamosa_db::db_query("SELECT app_id FROM {mediamosa_asset} WHERE asset_id = :asset_id", array(':asset_id' => $id))->fetchField();

        if (in_array($asset_id_app_id, $app_ids)) {
          break;
        }

        // Master/slave check
        // @todo: Hhmm in current context with rights on asset, this might not be the right approache.
        $db_result = mediamosa_db::db_query(
          "SELECT 1 FROM {mediamosa_asset} AS a
          JOIN {mediamosa_asset_mediafile} AS m ON a.asset_id = m.asset_id
          JOIN {mediamosa_acl_app_master_slave} AS ms ON ms.acl_object_type = 'MEDIAFILE'
            AND ms.acl_object_id = m.mediafile_id
            AND (ms.app_id_master IN (:app_id_master) OR ms.app_id_slave IN (:app_id_slave))
          WHERE a.asset_id = :asset_id",
          array(
            ':app_id_master' => $app_ids,
            ':app_id_slave' => $app_ids,
            ':asset_id' => $id,
          )
        );

        if ($db_result->fetchAssoc() === FALSE) {
          throw new mediamosa_exception_error_access_denied(array('@reason' => 'application not allowed'));
        }
        break;

      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        // Basic app_id check
        $mediafile_id_app_id = mediamosa_db::db_query("SELECT app_id FROM {mediamosa_asset_mediafile} WHERE mediafile_id = :mediafile_id", array(':mediafile_id' => $id))->fetchField();

        if (in_array($mediafile_id_app_id, $app_ids)) {
          break;
        }

        // Master/slave check.
        $db_result = mediamosa_db::db_query(
          "SELECT 1 FROM {mediamosa_asset_mediafile} AS m
          JOIN {mediamosa_acl_app_master_slave} AS ms ON ms.acl_object_type = 'MEDIAFILE' AND ms.acl_object_id = m.mediafile_id AND ms.app_id_master IN (:app_id_master)
          WHERE m.mediafile_id = :mediafile_id", array(
            ':app_id_master' => $app_ids,
            ':mediafile_id' => $id,
          )
        );

        if ($db_result->fetchAssoc() === FALSE) {
          throw new mediamosa_exception_error_access_denied(array('@reason' => 'application not allowed'));
        }
        break;

      default:
        throw new mediamosa_exception_error_access_denied(array('@reason' => 'application not allowed'));
    }
  }
}
