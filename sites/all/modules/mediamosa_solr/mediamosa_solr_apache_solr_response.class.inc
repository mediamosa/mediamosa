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

  /**
   * Convert the metadata field name from solr to MM field.
   *
   * @param string $solrfield
   *   The Solr field to convert.
   *
   * @return string
   *   The solr field.
   */
  protected function solrfield2metadatafield($solrfield) {
    return $solrfield;
  }

  /**
   * Get facets.
   *
   * In output from Solr we ignore;
   * - 'facet_query': Requires solr query, needs work (CQL -> solr).
   * - 'facet_range': Requires complex field specified input.
   *
   * @return array
   *   Related information. Key contains asset_id.
   *   - 'score': The score related.
   */
  public function getFacets() {
    $facets = array('solr_facet_fields' => array(), 'solr_facet_dates' => array());

    // facet_fields;
    // granted=true&calculate_total_count=true&solr_facet=true&solr_facet_field=title_vc

    // facet_dates;
    // granted=true&calculate_total_count=true&solr_facet=true&solr_facet_date=videotimestampmodified&solr_facet_date_start=NOW/DAY-5DAYS&solr_facet_date_end=NOW/DAY%2B1DAY&solr_facet_date_gap=%2B1DAY

    if (!empty($this->_response->facet_counts)) {
      if (!empty($this->_response->facet_counts->facet_fields)) {
        foreach ($this->_response->facet_counts->facet_fields as $field => $data) {
          $field = $this->solrfield2metadatafield($field);
          $facets['solr_facet_fields'][$field] = array();
          foreach ($data as $name => $value) {
            $facets['solr_facet_fields'][$field][$name] = (string) $value;
          }
        }

        foreach ($this->_response->facet_counts->facet_dates as $field => $dates) {
          $field = $this->solrfield2metadatafield($field);
          $facets['solr_facet_dates'][$field] = array();
          foreach ($dates as $name => $value) {
            $facets['solr_facet_dates'][$field][$name] = (string) $value;
          }
        }
      }
    }

    return $facets;
  }

  /**
   * Get related.
   *
   * @return array
   *   Related information. Key contains asset_id.
   *   - 'score': The score related.
   */
  public function getRelated() {
    $related = array();

    if (!empty($this->_response->moreLikeThis)) {
      foreach ($this->_response->moreLikeThis as $asset_id => $data) {
        $assets = array();
        foreach ($data->docs as $doc) {
          $assets[$doc->asset_id] = array('score' => $doc->score, 'asset_id' => $doc->asset_id);
          foreach ($doc as $name => $value) {
            if (mediamosa_unicode::substr($name, -3) == '_md') {
              $assets[$doc->asset_id]['metadata'][mediamosa_unicode::substr($name, 0, -3)] = $value;
            }
          }
        }

        $related[$asset_id] = array(
          'count_total' => (int) $data->numFound,
          'count' => count($assets),
          'offset' => (int) $data->start,
          'score_max' => (string) $data->maxScore,
          'assets' => $assets,
        );
      }
    }

    return $related;
  }
}
