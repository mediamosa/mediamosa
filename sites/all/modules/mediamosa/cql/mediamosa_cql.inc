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
 * cql               ::= cqlQuery ['sortby' sortSpec]
 * cqlQuery          ::= prefixAssignment cqlQuery | scopedClause
 * prefixAssignment  ::= '>' prefix '=' uri | '>' uri
 * scopedClause       ::= scopedClause booleanGroup searchClause | searchClause
 * booleanGroup       ::= boolean [modifierList]
 * boolean           ::= 'and' | 'or' | 'not' | 'prox'
 * searchClause      ::= '(' cqlQuery ')' | index relation searchTerm | searchTerm
 * relation          ::= comparitor [modifierList]
 * comparitor        ::= comparitorSymbol | namedComparitor
 * comparitorSymbol  ::= '=' | '>' | '<' | '>=' | '<=' | '<>' | '=='
 * namedComparitor   ::= identifier
 * modifierList      ::= modifierList modifier | modifier
 * modifier          ::= '/' modifierName [comparitorSymbol modifierValue]
 * sortSpec           ::= sortSpec singleSpec | singleSpec
 * singleSpec         ::= index [modifierList]
 * refix, uri,
 * modifierName,
 * modifierValue,
 * searchTerm, index ::= term
 * term              ::= identifier | 'and' | 'or' | 'not' | 'prox' | 'sortby'
 * identifier        ::= charString1 | charString2
 * charString1        := Any sequence of characters that does not include any of the following:
 *                       whitespace
 *                       ( (open parenthesis)
 *                       ) (close parenthesis)
 *                       =
 *                       <
 *                       >
 *                       '"' (double quote)
 *                       /
 * If the final sequence is a reserved word, that token is returned instead. Note that '.' (period)
 * may be included, and a sequence of digits is also permitted. Reserved words are 'and', 'or',
 * 'not', and 'prox' (case insensitive). When a reserved word is used in a search term,
 * case is preserved.
 *
 * charString2        := Souble quotes enclosing a sequence of any characters except double quote
 *                       (unless preceded by backslash (\)). Backslash escapes the character
 *                       following it. The resultant value includes all backslash characters except
 *                       those releasing a double quote (this allows other systems to interpret the
 *                       backslash character). The surrounding double quotes are not included.
 *
 *
 *
 * [[index] relation[/modifier]] [term] [sortBy index[/modifier]]
 */

/**
 * @file
 * Contains all non-drupal functions
 */

/**
 * Key defines for the context array
 */
define('MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES', 'indexes');// CQL index
define('MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS', 'columns');// SQL column
define('MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES', 'index_references'); // List of index => index, multiple indexes can be references
define('MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_RELATION', 'default_relation');// required
define('MEDIAMOSA_CQL_CONTEXT_KEY_DEFAULT_SEARCH_TERMS', 'default_search_terms');// required

define('MEDIAMOSA_CQL_BOOLEAN_OR', 'OR');
define('MEDIAMOSA_CQL_BOOLEAN_AND', 'AND');
define('MEDIAMOSA_CQL_BOOLEAN_NOT', 'NOT');
define('MEDIAMOSA_CQL_BOOLEAN_PROX', 'PROX');
