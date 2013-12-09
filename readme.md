MediaMosa is Open Source Software to build a Full Featured, Webservice
Oriented Media Management and Distribution platform (http://mediamosa.org)

Copyright (C) 2013 SURFnet BV (http://www.surfnet.nl) and Kennisnet
(http://www.kennisnet.nl)

MediaMosa is based on the open source Drupal platform and was originally
developed by Madcap BV (http://www.madcap.nl).

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program as the file LICENSE.txt; if not, please see
http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.

Join our official Google+ communitie
* https://plus.google.com/u/0/communities/108771938512877203754

# Using our source code from GitHub
MediaMosa 3.4 and higher now includes the MediaMosa CK module. The MediaMosa SDK
module is part of MediaMosa and is shared with any front end that wants to
develop for MediaMosa.

The following modules are now submodules;
* MediaMosa SDK
* MediaMosa CK

To complete a pull from GitHub, you must pull its submodules as well. You can do
this by using these git commands;

git submodule init  
git submodule update

If you re-pulling from MediaMosa, also include the following command to ensure
the submodule have the correct versions;

git submodule update
