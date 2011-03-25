<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

 /**
  * @file
  * Authorization layer for assets, media functions.
  */

class mediamosa_aut {

  // ------------------------------------------------------------------ Consts.
  const AUT_TYPE_MEDIAFILE = 'MEDIAFILE';
  const AUT_TYPE_ASSET = 'ASSET';
  const AUT_TYPE_COLLECTION = 'COLLECTION';

// Current rights
  const RIGHT_ACCESS = 'ACCESS';

  const AUT_NAME_TYPE_DOMAIN = 'DOMAIN';
  const AUT_NAME_TYPE_REALM = 'REALM';
  const AUT_NAME_TYPE_USER = 'USER';
  const AUT_NAME_TYPE_USER_GROUP = 'USER_GROUP';

// Dummy for old ACL layer.
  const AUT_NAME_TYPE_APP = 'APP';

// Dummy for result rights, name and PATH
  const OBJ_LINK_TYPE_APP = 'APP';
  const OBJ_LINK_TYPE_NAME = 'NAME';
  const OBJ_LINK_TYPE_GROUP = 'GROUP';

  // Copies from the table asset_mediafile column is_protected enum.
  const MEDIAFILE_IS_PROTECTED_FALSE = 'FALSE';
  const MEDIAFILE_IS_PROTECTED_TRUE = 'TRUE';
  const MEDIAFILE_IS_PROTECTED_USER_USERGROUP = 'USER_USERGROUP';
  const MEDIAFILE_IS_PROTECTED_DOMAIN_REALM = 'DOMAIN_REALM';

  // ------------------------------------------------------------------ Static Function.
  static public function valid_realm($realm) {
    assert($realm != '');
    return TRUE;
  }

  static public function valid_domain($domain) {
    assert($domain != '');
    return TRUE;
  }

  static public function is_group_domain($s_group_domain) {
    return (preg_match('/(.*)\.group$/', $s_group_domain) > 0 ? TRUE : FALSE);
  }

  static public function is_group_realm($s_group_realm) {
    return (preg_match('/@[^.]+\.group$/', $s_group_realm) > 0 ? TRUE : FALSE);
  }

  // Simple check if input is realm
  static public function is_realm($s_hostname) {
    return ((strpos($s_hostname, '@') === FALSE) ? FALSE : TRUE);
  }

  /**
   * Returns the aut_ids of the given domain and realms.
   *
   * @param integer/array $mixed_app_id
   * @param string $aut_user_id
   * @param array $a_aut_group_id
   *  These are user_id groups, like mf.group_id
   * @param array $aut_domain
   * @param array $aut_realm
   */
  static public function build_access($mixed_app_id, $aut_user_id = NULL, $a_aut_group_id = NULL, $aut_domain = NULL, $aut_realm = NULL, $a_slaves = NULL) {
    $a_query = array();

    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_aut_name}.aut_name_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_aut_name}.aut_group_id';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_aut_name}.aut_type';
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = '{mediamosa_aut_name}.app_id';

    // Make it an array.
    if (!is_array($mixed_app_id)) {
      $mixed_app_id = array($mixed_app_id);
    }

    // Prevent mysql choosing the uni index above the needed index.
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_aut_name}';

    assert(is_array($a_aut_group_id) || is_null($a_aut_group_id));

    // Make sure some value's are set to NULL when considered empty.
    $a_aut_group_id = empty($a_aut_group_id) ? NULL : $a_aut_group_id;
    $aut_domain = $aut_domain == '' ? NULL : $aut_domain;
    $aut_realm = $aut_realm == '' ? NULL : $aut_realm;

    // Make sure we do at least one of the wheres besides app_id, or else we will have access on all.
    $empty_access = TRUE;

    // First the app_id.
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['app_id'][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_aut_name}.app_id IN(%s)", implode(",", $mixed_app_id));

    // Do we have entry for our user_id and is not in a group (future exp. but lets do it now anyway)
    if (!is_null($aut_user_id) && $aut_user_id != "") {
      $empty_access = FALSE;

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['user'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_type = 'USER'";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['user'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_group_id IS NULL";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['user'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_aut_name}.aut_name = '%s'", mediamosa_db_query::query_escape($aut_user_id));
    }

    // Do we have entry for our user_id and is not in a group (group type is never in a group, we will not group groups here)
    if (!is_null($a_aut_group_id) && !empty($a_aut_group_id)) {
      // Make sure there are no 'empty' group_ids in our list, or else we will gain access by mistake
      foreach ($a_aut_group_id as $key => $s_group_id) {
        if (trim($s_group_id) == "") {
          unset($a_aut_group_id[$key]);
        }
      }

      if (!empty($a_aut_group_id)) {
        $empty_access = FALSE;

        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_type = 'USER_GROUP'";
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_group_id IS NULL";

        if (count($a_aut_group_id) == 1) {
            $s_group_id = reset($a_aut_group_id);
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_aut_name}.aut_name = '%s'", mediamosa_db_query::query_escape($s_group_id));
        }
        else {
          foreach ($a_aut_group_id as $s_group_id) {
            if (!empty($s_group_id)) {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND]['group'][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_aut_name}.aut_name = '%s'", mediamosa_db_query::query_escape($s_group_id));
            }
          }
        }
      }
    }

    // Does my domain match with any in the listing?
    if (!is_null($aut_domain)) {
      $empty_access = FALSE;

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_type='DOMAIN'";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND]['aut_group_id'][mediamosa_db_query::WHERE_OR][] = "{mediamosa_aut_name}.aut_group_id IS NULL";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND]['aut_group_id'][mediamosa_db_query::WHERE_OR][] = "{mediamosa_aut_name}.aut_group_id IS NOT NULL";

      // Split the domain on its '.'.
      $a_domain_parts = explode('.', $aut_domain);

      // Now reverse it.
      $a_domain_parts = array_reverse($a_domain_parts);

      // Now walk through and each time add one more.
      $a_domain_parts_to_join = array(reset($a_domain_parts));

      while (1) {
        $domain_part = next($a_domain_parts);

        $aut_domain = implode('.', array_reverse($a_domain_parts_to_join));
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['domain'][mediamosa_db_query::WHERE_AND]['aut_name'][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_aut_name}.aut_name = '%s'", mediamosa_db_query::query_escape($aut_domain));

        if ($domain_part === FALSE) {
          break; // from while.
        }

        $a_domain_parts_to_join[] = $domain_part; // next part plz.
      }

      // Domain type does not use prefix field so, we dont need to test IS NULL either.
    }

    if (!is_null($aut_realm)) {
      $empty_access = FALSE;

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_type='REALM'";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND]["aut_group_id"][mediamosa_db_query::WHERE_OR][] = "{mediamosa_aut_name}.aut_group_id IS NULL";
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND]["aut_group_id"][mediamosa_db_query::WHERE_OR][] = "{mediamosa_aut_name}.aut_group_id IS NOT NULL";

      // Realm check is special, very special

      // Must contain @, if not, include it
      if (strpos($aut_realm, '@') === FALSE) {
        $aut_realm = '@' . $aut_realm;
      }

      list($aut_prefix, $s_aut_domain) = explode('@', trim($aut_realm));
      if (trim($aut_prefix) == "") {
        $aut_prefix = NULL;
      }

      // do the same thing as we did on domain

      // Split the domain on its '.'
      $a_domain_parts = explode('.', $s_aut_domain);

      // Now reverse it
      $a_domain_parts = array_reverse($a_domain_parts);

      // Now walk through and each time add one more
      $a_domain_parts_to_join = array(reset($a_domain_parts));

      while (1) {
        $aut_realm = '@' . implode('.', array_reverse($a_domain_parts_to_join));

        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['aut_name'][mediamosa_db_query::WHERE_AND]["aut_prefix"][mediamosa_db_query::WHERE_OR][] = sprintf("{mediamosa_aut_name}.aut_name = '%s'", mediamosa_db_query::query_escape($aut_realm));

        $domain_part = next($a_domain_parts);
        if ($domain_part === FALSE) {
          break; // from while
        }

        $a_domain_parts_to_join[] = $domain_part;
      }

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['aut_name'][mediamosa_db_query::WHERE_AND][] = "{mediamosa_aut_name}.aut_prefix IS NULL";

      if (!is_null($aut_prefix)) {
        // if prefix was given we also check on the full [name] @ [domain] access
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['aut_name_full'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_aut_name}.aut_name = '%s'", mediamosa_db_query::query_escape('@' . $s_aut_domain));// its a realm, even though its called domain in its var name
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['realm'][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['aut_name_full'][mediamosa_db_query::WHERE_AND][] = sprintf("{mediamosa_aut_name}.aut_prefix = '%s'", mediamosa_db_query::query_escape($aut_prefix));
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
      if (intval($a_row['aut_group_id'])) {
        $a_result[$a_row['app_id']]['a_group'][] = $a_row['aut_group_id'];
      }
      else {
        $a_result[$a_row['app_id']]['a_name'][] = $a_row['aut_name_id'];
      }
    }

    $a_groups = array();
    foreach ($a_result as $app_id => $a_row) {
      // If no aut ids where found for this app, then remove it and continue.
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
       * $a_groups - contains all group ids of all the groups of all apps based on my aut settings.
       */

      // Collect all names.
      $query = mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag');
      $query->fields('ag', array(
        mediamosa_aut_group_db::ID,
        mediamosa_aut_group_db::AUT_GROUP_NAME,
        mediamosa_aut_group_db::AUT_GROUP_TYPE
      ));
      $query->condition(mediamosa_aut_group_db::ID, $a_groups, 'IN');

      /*
       * Now with all group names + types, find the groups of the other apps.
       */
      foreach ($query->execute() as $a_group) {
        $query_2 = mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag');
        $query_2->fields('ag', array(
          mediamosa_aut_group_db::APP_ID,
          mediamosa_aut_group_db::ID
        ));
        $query_2->condition(mediamosa_aut_group_db::AUT_GROUP_NAME, $a_group['aut_group_name']);
        $query_2->condition(mediamosa_aut_group_db::AUT_GROUP_TYPE, $a_group['aut_group_type']);

        // Loop through the results of muy group name + type search, so we find the groups of all apps.
        foreach ($query_2->execute() as $a_group_2) {
          // If app of found group is not one of my slaves, then I cant use it.
          if (!isset($a_slaves[$a_group_2['app_id']])) {
            continue;
          }

          // Group was found on one of my master/slaved apps.
          $a_result['slaves']['a_group'][$a_group_2['app_id']][$a_group_2['aut_group_id']] = $a_group_2['aut_group_id'];
        }

        // Get our app.
        $app_id = reset($mixed_app_id);

        // Get the app ids of our slaves.
        $a_app_id_slaves = array_keys($a_slaves);

        // Collect all names of the groups of our app.
        $query = mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag');
        $query->fields('ag', array(
          mediamosa_aut_group_db::ID,
          mediamosa_aut_group_db::AUT_GROUP_NAME,
          mediamosa_aut_group_db::AUT_GROUP_TYPE,
        ));
        $query->condition(mediamosa_aut_group_db::ID, $a_groups, 'IN');
        $query->condition(mediamosa_aut_group_db::APP_ID, $app_id);

        foreach ($query->execute() as $a_group) {
          // Collect all names of the groups of our app.
          $query_2 = mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag');
          $query_2->fields('ag', array(
            mediamosa_aut_group_db::APP_ID,
            mediamosa_aut_group_db::ID
          ));
          $query_2->condition(mediamosa_aut_group_db::AUT_GROUP_NAME, $a_group['aut_group_name']);
          $query_2->condition(mediamosa_aut_group_db::AUT_GROUP_TYPE, $a_group['aut_group_type']);
          $query_2->condition(mediamosa_aut_group_db::APP_ID, $app_id, '<>');

          // Now loop through all my groups that are not my app.
          foreach ($query_2->execute() as $a_group_2) {
            $query_3 = mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag');
            $query_3->fields('ag', array(
              mediamosa_aut_group_db::ID,
              mediamosa_aut_group_db::APP_ID,
            ));
            $query_3->condition(mediamosa_aut_group_db::AUT_GROUP_NAME, $a_group['aut_group_name']);
            $query_3->condition(mediamosa_aut_group_db::AUT_GROUP_TYPE, $a_group['aut_group_type']);
            $query_3->condition(mediamosa_aut_group_db::APP_ID, $a_app_id_slaves, 'IN');

            foreach ($query_3->execute() as $a_group_3) {
              $a_result['slaves_convert']['a_group'][$a_group_3['app_id']][$a_group_3['aut_group_id']] = $a_group_3['aut_group_id'];
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
  static public function owner_check($app_id, $user_id, $object_app_id, $user_id_owner, $is_app_admin = FALSE) {
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
   * @param string $aut_type
   *  The mediamosa_aut::AUT_TYPE_*
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
  static public function owner_check_ext($aut_type, $object_id, $app_id, $user_id, $object_app_id, $user_id_owner, $is_app_admin = FALSE) {
    try {
      mediamosa_aut::owner_check($app_id, $user_id, $object_app_id, $user_id_owner, $is_app_admin);
    }
    catch (mediamosa_exception_error_access_denied $e) {
      // Not owner allowed or not same app (or both).
      // We still have access to the object for reading rights if we are master.
      // So check if the object is a slave of us.

      // Get the slaves of the object using the App ID of the current EGA.
      $a_slaves = mediamosa_aut_app_master_slave::slave_get($app_id, $aut_type, $object_id);

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

  static public function owner_check_collection_assign($app_id, $user_id, array $asset, array $collection, $is_app_admin = FALSE) {
    // If the collection has public assign rights, then allow it
    if ($collection[mediamosa_collection_db::PUBLIC_ASSIGN] == mediamosa_collection_db::PUBLIC_ASSIGN_TRUE) {

      // Must be owner.
      try {
        mediamosa_aut::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);
        return; // We got access, no need to check collection ownership.
      }
      catch (mediamosa_exception_error_access_denied $e) {
        // Ignore, check ownership collection now.
        assert($e);
      }
    }

    mediamosa_aut::owner_check($app_id, $user_id, $collection[mediamosa_collection_db::APP_ID], $collection[mediamosa_collection_db::OWNER_ID], $is_app_admin);
  }

  /**
   * Test if you are app_admin
   *
   * @param integer $app_id
   * @param boolean $is_app_admin
   */
  static public function app_admin_check($app_id, $object_app_id, $is_app_admin) {
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
   * @param string $aut_type
   * @param array $a_object
   *  Make sure its an object that is within the same app_id as the user
   *  Or else it will be denied access
   * @param array $is_app_admin
   *  For future expansion
   *
   *  @return array($is_protected, $rights_set_user_usergroup, $rights_set_domain_realm)
   */
  static public function is_protected_get($app_id, $user_id, $aut_type, array $object, $is_app_admin = FALSE) {
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['mediafile_id'];
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['asset_id'];
        break;

      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    $is_protected = NULL;
    $rights_set_user_usergroup = FALSE;
    $rights_set_domain_realm = FALSE;

    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        // Need to be either owner + same app or slave of object.
        mediamosa_aut::owner_check_ext($aut_type, $aut_object_id, $app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

        $query = db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
        $query
          ->condition(mediamosa_asset_mediafile_db::ID, $aut_object_id)
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

      case mediamosa_aut::AUT_TYPE_ASSET:
        // Only owners (with the same app_id) can get access (for now)
        mediamosa_aut::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

        $query = db_select(mediamosa_asset_db::TABLE_NAME, 'mf');
        $query
          ->condition(mediamosa_asset_db::ID, $aut_object_id)
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
   * @param string $aut_type
   * @param array $object
   *  Make sure its an object that is within the same app_id as the user
   *  Or else it will be denied access
   * @param array $app_ids
   *  Reserved, not supported yet.
   * @param array $aut_user_ids
   * @param array $aut_group_ids
   * @param array $aut_domains
   * @param array $aut_realms
   * @param bool $replace
   * @param array $rights
   *  For future expansion
   * @param array $is_app_admin
   */
  static public function rights_set($app_id, $user_id, $aut_type, array $object, array $app_ids, array $aut_user_ids, array $aut_group_ids, array $aut_domains, array $aut_realms, $replace = TRUE, array $rights = array(mediamosa_aut::RIGHT_ACCESS), $is_app_admin = FALSE) {
    assert(count($rights));
    assert($aut_type == mediamosa_aut::AUT_TYPE_MEDIAFILE || $aut_type == mediamosa_aut::AUT_TYPE_ASSET || $aut_type == mediamosa_aut::AUT_TYPE_COLLECTION); // media files for now

    assert(!is_null($app_ids));
    assert(!is_null($aut_user_ids));
    assert(!is_null($aut_group_ids));
    assert(!is_null($aut_domains));
    assert(!is_null($aut_realms));

    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['mediafile_id'];
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['asset_id'];
        break;

      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Only owners (with the same app_id) can change access (for now)
    mediamosa_aut::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

    $rights_set = $rights_set_user_usergroup = $rights_set_domain_realm = FALSE;

    // Replace old existing rights.
    if ($replace) {
      mediamosa_aut::rights_clear($app_id, $user_id, $aut_type, $object, $is_app_admin);
    }
    else {
      // Get ACL rights, evaluate rights->isprotected.
      list($is_protected, $rights_set_user_usergroup, $rights_set_domain_realm) = mediamosa_aut::is_protected_get($app_id, $user_id, $aut_type, $object, $is_app_admin);
    }


    $result = array();

    // Link app access.
    // Master / Slave;
    // Only the owner can share his media to another app
    // The other app is then called 'master', and the media shared is the slave.
    foreach ($app_ids as $app_id_master) {

      try {
        mediamosa_aut_app_master_slave::create($aut_object_id, $aut_type, $app_id_master, $app_id);

        $result[] = array(
          'type' => mediamosa_aut::AUT_NAME_TYPE_APP,
          'value' => $app_id_master,
          'a_error' => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          'type' => mediamosa_aut::AUT_NAME_TYPE_APP,
          'value' => $app_id_master,
          'a_error' => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    // Link users access
    foreach ($aut_user_ids as $aut_user_id) {
      try {
        $aut_id = mediamosa_aut_name::create($app_id, $aut_user_id, mediamosa_aut::AUT_NAME_TYPE_USER);
        if (!$aut_id) {
          throw new mediamosa_exception_error_unexpected_error();
        }

        // Now link aut_name with object (using aut_object)
        mediamosa_aut_object::link_to_object($aut_object_id, mediamosa_aut_object_db::AUT_TYPE_NAME, $aut_type, $aut_id);

        $rights_set = $rights_set_user_usergroup = TRUE;
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_USER,
          "value" => $aut_user_id,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_USER,
          "value" => $aut_user_id,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    foreach ($aut_group_ids as $aut_group_id) {
      try {
        $aut_id = mediamosa_aut_name::create($app_id, $aut_group_id, mediamosa_aut::AUT_NAME_TYPE_USER_GROUP);
        assert($aut_id);
        if (!$aut_id) {
          throw new mediamosa_exception_error_unexpected_error();
        }

        // Now link aut_name with object (using aut_object)
        mediamosa_aut_object::link_to_object($aut_object_id, mediamosa_aut_object_db::AUT_TYPE_NAME, $aut_type, $aut_id);

        $rights_set = $rights_set_user_usergroup = TRUE;
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_USER_GROUP,
          "value" => $aut_group_id,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_USER_GROUP,
          "value" => $aut_group_id,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    foreach ($aut_domains as $aut_domain) {
      try {
        // Check if domain is group
        if (mediamosa_aut::is_group_domain($aut_domain)) { //[...].group domain
          $aut_id = mediamosa_aut_group::create($app_id, $aut_domain, mediamosa_aut::AUT_NAME_TYPE_DOMAIN);
          if (!$aut_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link aut_name with object (using aut_object)
          mediamosa_aut_object::link_to_object($aut_object_id, mediamosa_aut_object_db::AUT_TYPE_GROUP, $aut_type, $aut_id);
        }
        else {
          $aut_id = mediamosa_aut_name::create($app_id, $aut_domain, mediamosa_aut::AUT_NAME_TYPE_DOMAIN);
          if (!$aut_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link aut_name with object (using aut_object)
          mediamosa_aut_object::link_to_object($aut_object_id, mediamosa_aut_object_db::AUT_TYPE_NAME, $aut_type, $aut_id);
        }

        $rights_set = $rights_set_domain_realm = TRUE;
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_DOMAIN,
          "value" => $aut_domain,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_DOMAIN,
          "value" => $aut_domain,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    foreach ($aut_realms as $aut_realm) {
      try {
        // They can also define groups here...
        if (mediamosa_aut::is_group_realm($aut_realm)) { //@[...].group realm
          $aut_id = mediamosa_aut_group::create($app_id, $aut_realm, mediamosa_aut::AUT_NAME_TYPE_REALM);
          assert($aut_id);
          if (!$aut_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link aut_name with object (using aut_object)
          mediamosa_aut_object::link_to_object($aut_object_id, mediamosa_aut_object_db::AUT_TYPE_GROUP, $aut_type, $aut_id);
        }
        else {
          $aut_id = mediamosa_aut_name::create($app_id, $aut_realm, mediamosa_aut::AUT_NAME_TYPE_REALM);
          if (!$aut_id) {
            throw new mediamosa_exception_error_unexpected_error();
          }

          // Now link aut_name with object (using aut_object)
          mediamosa_aut_object::link_to_object($aut_object_id, mediamosa_aut_object_db::AUT_TYPE_NAME, $aut_type, $aut_id);
        }

        $rights_set = $rights_set_domain_realm = TRUE;
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_REALM,
          "value" => $aut_realm,
          "a_error" => FALSE,
        );
      }
      catch (mediamosa_exception_error $e) {
        $result[] = array(
          "type" => mediamosa_aut::AUT_NAME_TYPE_REALM,
          "value" => $aut_realm,
          "a_error" => $e->mediamosa_exception_error_array_get(),
        );
      }
    }

    if ($rights_set) {
      switch ($aut_type) {
        case mediamosa_aut::AUT_TYPE_MEDIAFILE:
          if (!$rights_set_domain_realm && $rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_aut::MEDIAFILE_IS_PROTECTED_USER_USERGROUP,
            );
          }
          elseif ($rights_set_domain_realm && !$rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM,
            );
          }
          else {
            $fields = array(
              mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE,
            );
          }

          // Add changed value.
          $fields = mediamosa_db::db_update_enrich($fields);

          // Update it.
          mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
            ->fields($fields)
            ->condition(mediamosa_asset_mediafile_db::ID, $aut_object_id)
            ->execute();

          // If its a still then remove the possible link to still.
          if (mediamosa_asset_mediafile::is_still($object)) {
            mediamosa_asset_mediafile_still::remove_permanent_link($aut_object_id);
          }
          else {
            // Remove all perm. still links to stills.
            $still_ids = mediamosa_asset_mediafile_still::find_asset_all_still($object[mediamosa_asset_mediafile_db::ASSET_ID]);

            foreach ($still_ids as $still_id) {
              mediamosa_asset_mediafile_still::remove_permanent_link($still_id);
            }
          }

          // Reindex the asset using the mediafile ID.
          mediamosa_asset_mediafile::mediamosa_asset_reindex(array($aut_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_AUT);
          break;

        case mediamosa_aut::AUT_TYPE_ASSET:
          if (!$rights_set_domain_realm && $rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_db::IS_PROTECTED => mediamosa_aut::MEDIAFILE_IS_PROTECTED_USER_USERGROUP,
            );
          }
          elseif ($rights_set_domain_realm && !$rights_set_user_usergroup) {
            $fields = array(
              mediamosa_asset_db::IS_PROTECTED => mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM,
            );
          }
          else {
            $fields = array(
              mediamosa_asset_db::IS_PROTECTED => mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE,
            );
          }

          // Add changed value.
          $fields = mediamosa_db::db_update_enrich($fields);

          // Update it.
          mediamosa_db::db_update(mediamosa_asset_db::TABLE_NAME)
            ->fields($fields)
            ->condition(mediamosa_asset_db::ID, $aut_object_id)
            ->execute();

          // Remove the permanent still links.
          $still_ids = mediamosa_asset_mediafile_still::find_asset_all_still($aut_object_id);
          foreach ($still_ids as $still_id) {
            mediamosa_asset_mediafile_still::remove_permanent_link($still_id);
          }

          // Reindex the asset.
          mediamosa_asset::mediamosa_asset_reindex(array($aut_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_AUT);
          break;

        case mediamosa_aut::AUT_TYPE_COLLECTION:
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
   * @param string $user_id
   * @param string $aut_type
   * @param array $object
   * @param boolean $is_app_admin
   */
  static public function rights_clear($app_id, $user_id, $aut_type, array $object, $is_app_admin = FALSE) {
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $owner_id = $object["owner_id"];
        $object_app_id = $object["app_id"];
        $aut_object_id = $object["mediafile_id"];
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        $owner_id = $object["owner_id"];
        $object_app_id = $object["app_id"];
        $aut_object_id = $object["asset_id"];
        break;

      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $owner_id = $object["owner_id"];
        $object_app_id = $object["app_id"];
        $aut_object_id = $object["coll_id"];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Only owners (with the same app_id) can change access (for now)
    mediamosa_aut::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);

    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $fields = array(
          mediamosa_asset_mediafile_db::IS_PROTECTED => mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE,
        );

        // Add changed value.
        $fields = mediamosa_db::db_update_enrich($fields);

        // Update it.
        mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
          ->fields($fields)
          ->condition(mediamosa_asset_mediafile_db::ID, $aut_object_id)
          ->execute();
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        $fields = array(
          mediamosa_asset_db::IS_PROTECTED => mediamosa_asset_db::IS_PROTECTED_FALSE,
        );

        // Add changed value.
        $fields = mediamosa_db::db_update_enrich($fields);

        // Update it.
        mediamosa_db::db_update(mediamosa_asset_db::TABLE_NAME)
          ->fields($fields)
          ->condition(mediamosa_asset_db::ID, $aut_object_id)
          ->execute();
        break;

      case mediamosa_aut::AUT_TYPE_COLLECTION:
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Remove all links between object and the aut_name/aut_group tabel.
    // This will result into removal of all rights.
    // Also remove the master / slave links
    mediamosa_db::db_query('DELETE FROM {mediamosa_aut_object} WHERE aut_object_type = :aut_object_type AND aut_object_id = :aut_object_id', array(
      ':aut_object_type' => $aut_type,
      ':aut_object_id' => $aut_object_id,
    ));

    // Remove the master / slave link, only the original app_id kan remove
    // these.
    mediamosa_db::db_query('DELETE FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type = :aut_object_type AND aut_object_id = :aut_object_id AND app_id_master > 0 AND app_id_slave = :app_id_slave', array(
      ':aut_object_type' => $aut_type,
      ':aut_object_id' => $aut_object_id,
      ':app_id_slave' => $app_id,
    ));

    // Reindex object.
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        // Reindex the asset using the mediafile ID.
        mediamosa_asset_mediafile::mediamosa_asset_reindex(array($aut_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_AUT);
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        // Reindex the asset using the mediafile ID.
        mediamosa_asset::mediamosa_asset_reindex(array($aut_object_id), mediamosa_settings::SEARCH_INDEX_TYPE_AUT);
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
   * @param string $aut_type
   * @param array $object
   * @param boolean $is_app_admin
   * @return array
   */
  static public function rights_get($app_id, $user_id, $aut_type, $object, $is_app_admin = FALSE) {
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['mediafile_id'];
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['asset_id'];
        break;

      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $owner_id = $object['owner_id'];
        $object_app_id = $object['app_id'];
        $aut_object_id = $object['coll_id'];
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        // Need to be either owner + same app or slave of object.
        mediamosa_aut::owner_check_ext($aut_type, $aut_object_id, $app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);
        break;

      default:
        // Only owners (with the same app_id) can get access (for now)
        mediamosa_aut::owner_check($app_id, $user_id, $object_app_id, $owner_id, $is_app_admin);
        break;
    }

    // Haal eerst de aut_id, zo we weten welke links er zijn
    $db_result = mediamosa_db::db_query(
      'SELECT #aut_id, #aut_type FROM {#mediamosa_aut_object} WHERE #aut_object_type = :aut_object_type AND #aut_object_id = :aut_object_id',
      array(
        '#mediamosa_aut_object' => mediamosa_aut_object_db::TABLE_NAME,
        '#aut_id' => mediamosa_aut_object_db::AUT_ID,
        '#aut_type' => mediamosa_aut_object_db::AUT_TYPE,
        '#aut_object_type' => mediamosa_aut_object_db::AUT_OBJECT_TYPE,
        ':aut_object_type' => $aut_type,
        '#aut_object_id' => mediamosa_aut_object_db::AUT_OBJECT_ID,
        ':aut_object_id' => $aut_object_id,
      )
    );

    $a_aut_ids = array();
    foreach ($db_result as $a_row) {
      $a_aut_ids[$a_row['aut_type']][] = $a_row['aut_id'];
    }

    $a_result = array(mediamosa_aut_object_db::AUT_TYPE_NAME => array(), mediamosa_aut_object_db::AUT_TYPE_GROUP => array(), mediamosa_aut::AUT_NAME_TYPE_APP => array());

    $db_result_name = $db_result_group = FALSE;

    if (isset($a_aut_ids[mediamosa_aut_object_db::AUT_TYPE_NAME]) && !empty($a_aut_ids[mediamosa_aut_object_db::AUT_TYPE_NAME])) {
      $db_result_name = mediamosa_db::db_select(mediamosa_aut_name_db::TABLE_NAME, 'an')
        ->fields('an')
        ->condition(mediamosa_aut_name_db::ID, $a_aut_ids[mediamosa_aut_object_db::AUT_TYPE_NAME], 'IN')
        ->execute();
    }

    if (isset($a_aut_ids[mediamosa_aut_object_db::AUT_TYPE_GROUP]) && !empty($a_aut_ids[mediamosa_aut_object_db::AUT_TYPE_GROUP])) {
      $db_result_group = mediamosa_db::db_select(mediamosa_aut_group_db::TABLE_NAME, 'ag')
        ->fields('ag')
        ->condition(mediamosa_aut_group_db::ID, $a_aut_ids[mediamosa_aut_object_db::AUT_TYPE_GROUP], 'IN')
        ->execute();
    }

    // Master / slave(s)
    // Note / todo ;
    // does the owner of the mediafile have the right to see other slave rights from other master app (not his app)?
    // For now we only allow rights that owner might have set (he cant set or remove app slave rights if he/she does not have the same master app
    $db_result_master_slave = mediamosa_db::db_query("SELECT * FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type = :aut_object_type AND aut_object_id = :aut_object_id AND app_id_master > 0 AND app_id_slave = :app_id_slave", array(
      ':aut_object_type' => $aut_type,
      ':aut_object_id' => $aut_object_id,
      ':app_id_slave' => $object_app_id,
    ));

    if ($db_result_name) {
      foreach ($db_result_name as $a_row) {
        $name = $a_row['aut_name'];
        if ($a_row['aut_type'] == mediamosa_aut::AUT_NAME_TYPE_REALM)  {
          $name = $a_row['aut_prefix'] . $a_row['aut_name'];
        }

        $a_result[mediamosa_aut_object_db::AUT_TYPE_NAME][] = array('type' => 'aut_' . mediamosa_unicode::strtolower($a_row['aut_type']), 'name' => $name);
      }
    }

    if ($db_result_group) {
      foreach ($db_result_group as $a_row) {
        $a_result[mediamosa_aut_object_db::AUT_TYPE_GROUP][] = array('type' => 'aut_' . mediamosa_unicode::strtolower($a_row['aut_group_type']), 'name' => $a_row['aut_group_name']);
      }
    }

    // Master / slave
    // Master is de share, zodat master onze object mag zien
    foreach ($db_result_master_slave as $a_master_slave) {
      $a_result[mediamosa_aut::AUT_NAME_TYPE_APP][] = array('type' => 'aut_app', 'app_id_slave' => $a_master_slave['app_id_slave'], 'app_id_master' => $a_master_slave['app_id_master']);
    }

    return $a_result;
  }

  /**
   * Copy the ACL rights, slave/master from one object to another, does not have to be the same object type
   * Remember that will add right(s), does not remove current rights.
   *
   * @param string $aut_type_source
   * @param string $aut_id_source
   * @param string $aut_type_dest
   * @param string $aut_id_dest
   */
  static public function replace_obj_to_obj($aut_type_source, $aut_id_source, $aut_type_dest, $aut_id_dest) {

    // We replace, so remove any existing rights 1st.
    mediamosa_db::db_query("DELETE FROM {mediamosa_aut_object} WHERE aut_object_type=:aut_object_type AND aut_object_id=:aut_object_id", array(
      ':aut_object_type' => $aut_type_dest,
      ':aut_object_id' => $aut_id_dest,
    ));
    mediamosa_db::db_query("DELETE FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type=:aut_object_type AND aut_object_id=:aut_object_id", array(
      ':aut_object_type' => $aut_type_dest,
      ':aut_object_id' => $aut_id_dest,
    ));

    // Copy normal ACL rights
    $db_result = mediamosa_db::db_query("SELECT aut_type, aut_id FROM {mediamosa_aut_object} WHERE aut_object_type = :aut_object_type AND aut_object_id = :aut_object_id", array(
      ':aut_object_type' => $aut_type_source,
      ':aut_object_id' => $aut_id_source,
    ));
    foreach ($db_result as $dbrow) {
      mediamosa_db::db_query("INSERT INTO {mediamosa_aut_object} SET aut_object_id = :aut_object_id, aut_type = :aut_type, aut_id = :aut_id, aut_object_type = :aut_object_type", array(
        ':aut_object_id' => $aut_id_dest,
        ':aut_type' => $dbrow['aut_type'],
        ':aut_id' => $dbrow['aut_id'],
        ':aut_object_type' => $aut_type_dest,
      ));
    }

    // Copy master/slave settings
    $db_result = mediamosa_db::db_query("SELECT app_id_slave, app_id_master FROM {mediamosa_aut_app_master_slave} WHERE aut_object_type = :aut_object_type AND aut_object_id = :aut_object_id", array(
      ':aut_object_type' => $aut_type_source,
      ':aut_object_id' => $aut_id_source,
    ));
    foreach ($db_result as $dbrow) {
      mediamosa_db::db_query("INSERT INTO {mediamosa_aut_app_master_slave} SET aut_object_id = :aut_object_id, aut_object_type = :aut_object_type, app_id_slave = :app_id_slave, app_id_master=:app_id_master", array(
        ':aut_object_id' => $aut_id_dest,
        ':aut_object_type' => $aut_type_dest,
        ':app_id_slave' => $dbrow['app_id_slave'],
        ':app_id_master' => $dbrow['app_id_master'],
      ));
    }

    // Now copy the is_protected from source to dest.
    switch ($aut_type_source) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $is_protected = mediamosa_db::db_query("SELECT is_protected FROM {mediamosa_asset_mediafile} WHERE mediafile_id = :mediafile_id", array(':mediafile_id' => $aut_id_source))->fetchAssoc();
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        $is_protected = mediamosa_db::db_query("SELECT is_protected FROM {mediamosa_asset} WHERE asset_id = :asset_id", array(':asset_id' => $aut_id_source))->fetchAssoc();
        break;

      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Now update the new object with the copy is_protected.
    switch ($aut_type_source) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        mediamosa_db::db_query("UPDATE {mediamosa_asset_mediafile} SET is_protected = :is_protected WHERE mediafile_id = :mediafile_id", array(
          ':is_protected' => $is_protected,
          ':mediafile_id' => $aut_id_dest,
        ));

        // Reindex the asset using the mediafile ID.
        mediamosa_asset_mediafile::mediamosa_asset_reindex(array($aut_id_dest), mediamosa_settings::SEARCH_INDEX_TYPE_AUT);
        break;

      case mediamosa_aut::AUT_TYPE_ASSET:
        mediamosa_db::db_query("UPDATE {mediamosa_asset} SET is_protected = :is_protected WHERE asset_id = :asset_id", array(
          ':is_protected' => $is_protected,
          ':asset_id' => $aut_id_dest,
        ));

        // Reindex the asset using the mediafile ID.
        mediamosa_asset::mediamosa_asset_reindex(array($aut_id_dest), mediamosa_settings::SEARCH_INDEX_TYPE_AUT);
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
  static public function replace_mediafile_to_mediafile($mediafile_id_src, $mediafile_id_dest) {
    return mediamosa_aut::replace_obj_to_obj(mediamosa_aut::AUT_TYPE_MEDIAFILE, $mediafile_id_src, mediamosa_aut::AUT_TYPE_MEDIAFILE, $mediafile_id_dest);
  }

  /**
   * Creates the sql to include in your sql call on object to test access
   * 1.x: vpx_acl_build_access_where
   *
   * @param string $aut_type
   *  AUT_TYPE_* ID
   * @param array $a_query
   * @param string $aut_type
   *   see AUT_TYPE_*
   * @param integer $object_id
   *  If given, we check only on this object for rights.
   *  This is also the main switch between a search query and an object search
   *  Specify NULL for non-direct matching.
   * @param integer $app_id
   * @param string $aut_user_id
   * @param array $aut_group_ids
   * @param string $aut_domain
   * @param string $aut_realm
   * @param array $a_aut_ids
   * @param bool $b_is_slave
   *  optional when providing own auth_ids
   *
   * @param bool $do_master_slave_null
   *  In some cases, when row is NULL (no rows), we still want to include it.
   * @param string $app_id_table_prefix
   *  In some cases we want another table being used for app_id prefix.
   *
   */
  static public function build_access_where(&$a_query, $aut_type, $object_id, $mixed_app_id, $aut_user_id, $aut_group_ids, $aut_domain, $aut_realm, array $slaves = array(), $is_app_admin = FALSE, $do_master_slave_only = FALSE, $do_master_slave_null = FALSE, $app_id_table_prefix = 'a') {
    assert(is_bool($is_app_admin));

    $a_app_ids = $mixed_app_id;
    if (!is_array($a_app_ids)) {
      $a_app_ids = array($a_app_ids);
    }

    if (empty($slaves)) {
      $slaves = mediamosa_aut_app_master_slave::slave_get($a_app_ids, $aut_type);
    }

    // Also require the rights from the other possible slave app(s)
    $a_app_ids_tmp = array_unique(array_merge($a_app_ids, array_keys($slaves)));

    // Get of this current user the aut_name/aut_group ids so we know his/her access.
    $a_aut_ids = mediamosa_aut::build_access($a_app_ids_tmp, $aut_user_id, $aut_group_ids, $aut_domain, $aut_realm, $slaves);

    if (!is_null($aut_user_id) || (!is_null($aut_group_ids) && !empty($aut_group_ids)) || count($a_aut_ids) || count($slaves)) {
      switch ($aut_type) {
        case mediamosa_aut::AUT_TYPE_MEDIAFILE:

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id = mf.asset_id";
          // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps
          if (!$do_master_slave_only) {
            // If $do_master_slave_only is TRUE, granted is TRUE, meaning we need to return all assets
            // even if we dont have access. This switch prevents returning all assets of the other
            // slave apps.

            // If we need to check aut_object
            if (count($a_aut_ids)) {
              $a_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get($aut_type);
            }

            // allow assets without mediafile for master app_id
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = sprintf('a.app_id IN(%s)', implode(',', $a_app_ids));
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = 'mf.asset_id IS NULL';

            if (is_null($object_id)) {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file='TRUE'";
            }
            else {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.mediafile_id='" . mediamosa_db::escape_string($object_id) . "'";
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = 'mf.is_original_file IS NOT NULL';// always TRUE, because its never NULL
            }

            // If we have aut id for the master app, check them
            foreach ($a_app_ids as $app_id) {
              if (!empty($a_aut_ids[$app_id])) {
                // Master app_id checks
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "mf.app_id=" . mediamosa_db::escape_string($app_id);

                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR][] = "mf.is_protected = 'FALSE'";

                // RBL: Removed, because is_protected is always != FALSE when aut rules apply.
                //$a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = "mf.is_protected != 'FALSE'";

                if (count($a_aut_ids[$app_id]['a_name'])) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_type = 'NAME'";
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $a_aut_ids[$app_id]['a_name']) . ")";
                }

                if (count($a_aut_ids[$app_id]['a_group'])/* || (isset($a_aut_ids['slaves']) && isset($a_aut_ids['slaves']['a_group'][$app_id]))*/) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_type = 'GROUP'";
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $a_aut_ids[$app_id]['a_group']) . ")";
                }
              }
              else {
                // allow master app_id media when not protected only
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "mf.app_id=" . mediamosa_db::escape_string($app_id);
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR][] = "mf.is_protected = 'FALSE'";
              }

              if (!is_null($aut_user_id)) {
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]['user_id'] = "mf.owner_id = '" . mediamosa_db::escape_string($aut_user_id) . "'";
              }
              // group_id
              if (!is_null($aut_group_ids)) {
                assert(is_array($aut_group_ids));

                foreach ($aut_group_ids as $s_group_id) {
                  if (!empty($s_group_id)) {
                    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]['group_id'][] = "mf.group_id = '" . mediamosa_db::escape_string($s_group_id) . "'";
                  }
                }
              }
            }
          }
          else {
            // When $do_master_slave_only is TRUE, we have always access to media of our own app_id(s)

            // Master app_id checks
            // Because the $a_app_ids is in the app_id selection of assets, this one is a bit useles...
            if (count($a_app_ids)) {
              // Master app_id checks
              if ($do_master_slave_null) {
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR]['master_app_id'][mediamosa_db_query::WHERE_AND]['ms'] = strtr('@app_id_table_prefix.app_id IN(@app_ids) OR @app_id_table_prefix.app_id IS NULL', array('@app_id_table_prefix' => $app_id_table_prefix, '@app_ids' => implode(',', $a_app_ids)));
              }
              else {
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR]['master_app_id'][mediamosa_db_query::WHERE_AND]['ms'] = strtr('@app_id_table_prefix.app_id IN(@app_ids)', array('@app_id_table_prefix' => $app_id_table_prefix, '@app_ids' => implode(',', $a_app_ids)));
              }
            }
          }

          // possible slaves
          if (count($slaves)) {
            $a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave'] = mediamosa_aut_app_master_slave::join_aut_app_master_slave_get($aut_type);

            // If we need to check aut_object
            if (count($a_aut_ids)) {
              $a_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get($aut_type);
            }

            foreach ($slaves as $app_id_slave => $a_slave) {
              assert($app_id_slave);// should not happen
              // Only for app_ids that are present

              // Must be same app_id for slaves
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = $app_id_table_prefix . '.app_id=' . mediamosa_db::escape_string($app_id_slave);

              // Any case, a link to app must be present if we allow access to the MF
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("aut_ms.app_id_master IN(%s)", implode(",", $a_app_ids));
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("aut_ms.app_id_slave = %d", $app_id_slave);

              if ($do_master_slave_only) {
                // If we only do master/slave, we include all that has been slaved and protected
                continue;
              }

              // 2 cases;
              // 1. The object is protected only app_id (is_protected == FALSE)
              //   then only access if app_id == slave_app_id
              // 2. The object is protected by other rights (is_protected == TRUE)
              //   then only access if app_id == slave_app_id and rights match with other rights

              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

              if (isset($a_aut_ids[$app_id_slave]) && count($a_aut_ids[$app_id_slave])) {
                if (count($a_aut_ids[$app_id_slave]['a_name'])) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
                }

                // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
                if (count($a_aut_ids[$app_id_slave]['a_name'])) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_type='NAME'";
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $a_aut_ids[$app_id_slave]['a_name']) . ")";
                }
              }

              if (isset($a_aut_ids['slaves_convert']) && isset($a_aut_ids['slaves_convert']['a_group'][$app_id_slave])) {
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_type='GROUP'";
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $a_aut_ids['slaves_convert']['a_group'][$app_id_slave]) . ")";
              }
            }
          }

          break;

        case mediamosa_aut::AUT_TYPE_ASSET:

          // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps
          if (!$do_master_slave_only) {
            // If $do_master_slave_only is TRUE, granted is TRUE, meaning we need to return all assets
            // even if we dont have access. This switch prevents returning all assets of the other
            // slave apps.

            // If we need to check aut_object
            if (count($a_aut_ids)) {
              $a_query[mediamosa_db_query::A_JOIN]['aut_object_asset'] = mediamosa_aut_object::join_aut_object_get($aut_type);
            }
/*
            // allow assets without mediafile for master app_id
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = sprintf('a.app_id IN(%s)', implode(',', $a_app_ids));
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]["no_mf"][mediamosa_db_query::WHERE_AND][] = 'mf.asset_id IS NULL';
/*
 *
 * Hmmm hmmm
            if (is_null($object_id)) {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file='TRUE'";
            }
            else {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = "mf.mediafile_id='" . mediamosa_db::escape_string($object_id) . "'";
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['mf'][mediamosa_db_query::WHERE_AND][] = 'mf.is_original_file IS NOT NULL';// always TRUE, because its never NULL
            }
 */
            // If we have aut id for the master app, check them
            foreach ($a_app_ids as $app_id) {
              if (!empty($a_aut_ids[$app_id])) {
                // Master app_id checks
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "a.app_id=" . mediamosa_db::escape_string($app_id);

                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR][] = "a.is_protected = 'FALSE'";
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = "a.is_protected != 'FALSE'";

                if (!empty($a_aut_ids[$app_id]['a_name'])) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_type = 'NAME'";
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_id IN(" . implode(",", $a_aut_ids[$app_id]['a_name']) . ")";
                }

                if (!empty($a_aut_ids[$app_id]['a_group']) || !empty($a_aut_ids['slaves']['a_group'][$app_id])) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_type = 'GROUP'";
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_id IN(" . implode(",", $a_aut_ids[$app_id]['a_group']) . ")";
                }
              }
              else {
                // allow master app_id media when not protected only
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND][] = "a.app_id=" . mediamosa_db::escape_string($app_id);
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR][] = "a.is_protected = 'FALSE'";
              }

              if (!is_null($aut_user_id)) {
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]['user_id'] = "a.owner_id = '" . mediamosa_db::escape_string($aut_user_id) . "'";
              }

              // group_id
              if (!is_null($aut_group_ids)) {
                assert(is_array($aut_group_ids));

                foreach ($aut_group_ids as $s_group_id) {
                  if (!empty($s_group_id)) {
                    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]['group_id'][] = "a.group_id = '" . mediamosa_db::escape_string($s_group_id) . "'";
                  }
                }
              }
            }
          }
          else {
            // When $do_master_slave_only is TRUE, we have always access to media of our own app_id(s)

            // Master app_id checks
            // Because the $a_app_ids is in the app_id selection of assets, this one is a bit useles...
            if (count($a_app_ids)) {
            // Master app_id checks
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR]['master_app_id'][mediamosa_db_query::WHERE_AND]['ms'] = sprintf('a.app_id IN(%s)', implode(',' , $a_app_ids));
            }
          }

          // possible slaves
          if (count($slaves)) {
            $a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave_asset'] = mediamosa_aut_app_master_slave::join_aut_app_master_slave_get($aut_type);

            // If we need to check aut_object
            if (count($a_aut_ids)) {
              $a_query[mediamosa_db_query::A_JOIN]['aut_object_asset'] = mediamosa_aut_object::join_aut_object_get($aut_type);
            }

            foreach ($slaves as $app_id_slave => $a_slave) {
              assert($app_id_slave);// should not happen
              // Only for app_ids that are present

              // Must be same app_id for slaves
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = "a.app_id ='" . mediamosa_db::escape_string($app_id_slave);

              // Any case, a link to app must be present if we allow access to the MF
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("aut_msa.app_id_master IN(%s)", implode(",", $a_app_ids));
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf("aut_msa.app_id_slave = %d", $app_id_slave);

              if ($do_master_slave_only) {
                // If we only do master/slave, we include all that has been slaved and protected
                continue;
              }

              // 2 cases;
              // 1. The object is protected only app_id (is_protected == FALSE)
              //   then only access if app_id == slave_app_id
              // 2. The object is protected by other rights (is_protected == TRUE)
              //   then only access if app_id == slave_app_id and rights match with other rights

              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

              if (isset($a_aut_ids[$app_id_slave]) && count($a_aut_ids[$app_id_slave])) {
                // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
                if (count($a_aut_ids[$app_id_slave]['a_name'])) {
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_type='NAME'";
                  $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['name'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_id IN(" . implode(",", $a_aut_ids[$app_id_slave]['a_name']) . ")";
                }
              }

              if (isset($a_aut_ids['slaves_convert']) && isset($a_aut_ids['slaves_convert']['a_group'][$app_id_slave])) {
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][] = sprintf("(a.is_protected='%s' OR a.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_type='GROUP'";
                $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['a'][mediamosa_db_query::WHERE_AND]['org'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND]["is_protected"][mediamosa_db_query::WHERE_OR]["is_protected_TRUE"][mediamosa_db_query::WHERE_AND][mediamosa_db_query::WHERE_OR]['group'][mediamosa_db_query::WHERE_AND][] = "aut_obja.aut_id IN(" . implode(",", $a_aut_ids['slaves_convert']['a_group'][$app_id_slave]) . ")";
              }
            }
          }
          break;

        case mediamosa_aut::AUT_TYPE_COLLECTION:
          // Allow master app always
          $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][0][mediamosa_db_query::WHERE_AND][] = sprintf('c.app_id IN(%s)', implode(',', $a_app_ids));

          if (count($slaves)) {
            $a_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave_coll'] = mediamosa_aut_app_master_slave::join_aut_app_master_slave_get($aut_type);

            foreach ($slaves as $app_id_slave => $a_slave) {
              // Must be same app_id for slaves
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = 'c.app_id = ' . mediamosa_db::escape_string($app_id_slave);

              // Any case, a link to app must be present if we allow access to the MF
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf('aut_msc.app_id_master IN(%s)', implode(',', $a_app_ids));
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access_coll'][mediamosa_db_query::WHERE_OR][$app_id_slave][mediamosa_db_query::WHERE_AND][] = sprintf('aut_msc.app_id_slave = %d', $app_id_slave);
            }
          }
          break;

        default:
          assert(0);
          return;
      }
    }
    else {
      switch ($aut_type) {
        case mediamosa_aut::AUT_TYPE_MEDIAFILE:
          // Include the mediafile table for join
          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id = mf.asset_id";

          $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['no_mf'] = "mf.asset_id IS NULL";
          $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id'][mediamosa_db_query::WHERE_AND][] = "mf.is_original_file = 'TRUE'";
          $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id'][mediamosa_db_query::WHERE_AND][] = sprintf("mf.app_id IN(%s)", implode(',', $a_app_ids));

          if (!$do_master_slave_only) {
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id'][mediamosa_db_query::WHERE_AND][] = "mf.is_protected = 'FALSE'";
          }
          break;

        case mediamosa_aut::AUT_TYPE_ASSET:
          $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id_asset'][mediamosa_db_query::WHERE_AND][] = sprintf('a.app_id IN(%s)', implode(',', $a_app_ids));

          if (!$do_master_slave_only) {
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['access'][mediamosa_db_query::WHERE_OR]['app_id_asset'][mediamosa_db_query::WHERE_AND][] = "a.is_protected = 'FALSE'";
          }
          break;

        case mediamosa_aut::AUT_TYPE_COLLECTION:
          $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection'][] = sprintf('c.app_id IN(%s)', implode(',', $a_app_ids));

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
   * @param string $aut_user_id
   * @param array $aut_group_ids
   * @param string $s_aut_domain
   * @param string $s_aut_realm
   * @return array
   *  returns the array with asset_id(s) on which we have access on
   */
  static public function access_check_assets($asset_ids, array $app_ids, $aut_user_id, array $aut_group_ids, $aut_domain, $aut_realm, $is_app_admin = FALSE) {

    $query = array();

    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.asset_id';
    $query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset} AS a';

    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id IN('%s')", implode("','", $asset_ids));

    // Build the access where.
    mediamosa_aut::build_access_where($query, mediamosa_aut::AUT_TYPE_MEDIAFILE, NULL, $app_ids, $aut_user_id, $aut_group_ids, $aut_domain, $aut_realm, array(), $is_app_admin);

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
   * @param string $aut_type
   * @param string $asset_id
   * @param string $object_id
   * @param integer $app_id
   * @param string $aut_user_id
   * @param array $aut_group_ids
   * @param string $s_aut_domain
   * @param string $s_aut_realm
   *
   * @throws mediamosa_exception_error_access_denied (ERRORCODE_NOT_AUTHORIZED) when access is denied
   */
  static public function access_check_on_object($aut_type, $asset_id, $object_id, $app_id, $aut_user_id, $aut_group_ids, $s_aut_domain, $s_aut_realm, $is_app_admin = FALSE) {

    if ($is_app_admin) {
      return;
    }

    assert(!is_null($object_id));
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:

        $query = array();

        $query[mediamosa_db_query::A_SELECT_EXPR][] = "a.asset_id";
        $query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

        // Build the access where
        mediamosa_aut::build_access_where($query, $aut_type, $object_id, $app_id, $aut_user_id, $aut_group_ids, $s_aut_domain, $s_aut_realm, array(), $is_app_admin);

        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id = '%s'", $asset_id);
        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

        if (!$is_app_admin) {
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = "a.is_unappropriate = 'FALSE'";

          if ($aut_user_id) {// if provided, then we must be owner to access unappropiate assets
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = sprintf("(a.is_unappropriate = 'TRUE' AND a.owner_id='%s')", mediamosa_db::escape_string($aut_user_id));
          }
        }

        // Convert into SQL.
        $query_sql = mediamosa_db_query::query_select($query);
mediamosa_debug::log_export($query_sql);
        // Now execute the SQL.
        $db_result = mediamosa_db::db_query($query_sql);

        if ($db_result->fetchAssoc() === FALSE) {
          // FIXME: will always be thrown, even when unappropriate was the reason.
          throw new mediamosa_exception_error_access_denied(array('@reason' => 'mediafile is protected'));
        }

        break;

      case mediamosa_aut::AUT_TYPE_ASSET:

        $query = array();

        $query[mediamosa_db_query::A_SELECT_EXPR][] = "a.asset_id";
        $query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

        // Build the access where
        mediamosa_aut::build_access_where($query, $aut_type, $object_id, $app_id, $aut_user_id, $aut_group_ids, $s_aut_domain, $s_aut_realm, array(), $is_app_admin);

        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id ='%s'", $asset_id);
        $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = "(a.parent_id IS NOT NULL OR a.parent_id IS NULL)";

        if (!$is_app_admin) {
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = "a.is_unappropriate = 'FALSE'";

          if ($aut_user_id) {// if provided, then we must be owner to access unappropiate assets
            $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_OR][] = sprintf("(a.is_unappropriate = 'TRUE' AND a.owner_id = '%s')", mediamosa_db::escape_string($aut_user_id));
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
  static public function read_single_object($type, $id, array $a_app_ids) {

    switch ($type) {
      case mediamosa_aut::AUT_TYPE_ASSET:

        // Basic app_id check
        $asset_id_app_id = mediamosa_db::db_query("SELECT app_id FROM {mediamosa_asset} WHERE asset_id = :asset_id", array(':asset_id' => $id))->fetchField();

        if (in_array($asset_id_app_id, $a_app_ids)) {
          break;
        }

        // Master/slave check
        // @todo: Hhmm in current context with rights on asset, this might not be the right approache.
        $db_result = mediamosa_db::db_query(
          "SELECT 1 FROM {mediamosa_asset} AS a
          JOIN {mediamosa_asset_mediafile} AS m ON a.asset_id = m.asset_id
          JOIN {mediamosa_aut_app_master_slave} AS ms ON ms.aut_object_type = 'MEDIAFILE'
            AND ms.aut_object_id = m.mediafile_id
            AND (ms.app_id_master IN (:app_id_master) OR ms.app_id_slave IN (:app_id_slave))
          WHERE a.asset_id = :asset_id",
          array(
            ':app_id_master' => $a_app_ids,
            ':app_id_slave' => $a_app_ids,
            ':asset_id' => $id,
          )
        );

        if ($db_result->fetchAssoc() === FALSE) {
          throw new mediamosa_exception_error_access_denied(array('@reason' => 'application not allowed'));
        }
        break;

      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        // Basic app_id check
        $mediafile_id_app_id = mediamosa_db::db_query("SELECT app_id FROM {mediamosa_asset_mediafile} WHERE mediafile_id = :mediafile_id", array(':mediafile_id' => $id))->fetchField();

        if (in_array($mediafile_id_app_id, $a_app_ids)) {
          break;
        }

        // Master/slave check.
        $db_result = mediamosa_db::db_query(
          "SELECT 1 FROM {mediamosa_asset_mediafile} AS m
          JOIN {mediamosa_aut_app_master_slave} AS ms ON ms.aut_object_type = 'MEDIAFILE' AND ms.aut_object_id = m.mediafile_id AND ms.app_id_master IN (:app_id_master)
          WHERE m.mediafile_id = :mediafile_id", array(
            ':app_id_master' => $a_app_ids,
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
