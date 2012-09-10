<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * REST calls specific for use inside the simpletest sandbox.
  */

/**
 * URI: /cron/solr
 *
 * This url is triggered by the cron.
 *
 * Internal only.
 *
 */
class mediamosa_rest_call_solr_process_queue extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Reindex what we can.
    mediamosa_solr::processQueue();

    // Add info to output.
    $mediamosa->set_result_okay();
  }
}
