<?php
/**
 * @file
 * Wrapper around Apache_Solr_Response.
 */

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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

class mediamosa_solr_apache_solr_response {
  /**
   * The decoded json response.
   *
   * @var object
   */
  private $_response = NULL;

  // -------------------------------------------------------------- Constructor.
  public function __construct(Apache_Solr_Response $response) {
    $this->_response = json_decode($response->getRawResponse());
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the decoded json response.
   *
   * @return object
   *   The decoded json response.
   */
  public function getResponse() {
    return $this->_response;
  }

  /**
   * Get total found rows, is not the number of found rows, but total matched
   * to the query.
   *
   * @return integer
   *   The number of found rows.
   */
  public function getNumFound() {
    return empty($this->_response->response->numFound) ? 0 : (int) $this->_response->response->numFound;
  }

  /**
   * Get the asset ids.
   *
   * @return array
   *   Array of found asset IDs.
   */
  public function getAssetIds() {
    $asset_ids = array();

    if (!empty($this->_response->response->docs)) {
      foreach ($this->_response->response->docs as $doc) {
        $asset_ids[] = (string) $doc->asset_id;
      }
    }

    return $asset_ids;
  }
}
