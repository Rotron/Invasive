Invasive
==========

An aggressive AX.25 decoder.

![head](http://h2so5.net/wp-content/uploads/2014/06/invasive_scr.png)

## Feature

### Socket Server

JSON-encoded frame data is available on TCP port 19780 (by default)
```
{"addresses":[{"callsign":"SS5PPQ","repeated":false,"ssid":2},{"callsign":"AE6MP","repeated":false,"ssid":0},{"callsign":"N6EX","repeated":true,"ssid":1}],"datetime":"2014-06-14T22:17:11","info-ascii":"`.](n->>/\"4W}","info-hex":"602e5d286e2d3e3e2f2234577d"}
{"addresses":[{"callsign":"SS5PPQ","repeated":false,"ssid":1},{"callsign":"AE6MP","repeated":false,"ssid":0},{"callsign":"N6EX","repeated":true,"ssid":5}],"datetime":"2014-06-14T22:17:12","info-ascii":"`.](n->>/\"4W}","info-hex":"602e5d286e2d3e3e2f2234577d"}
{"addresses":[{"callsign":"GPSLJ","repeated":false,"ssid":0},{"callsign":"N6QFD","repeated":false,"ssid":9},{"callsign":"N6EX","repeated":true,"ssid":5}],"datetime":"2014-06-14T22:17:18","info-ascii":"$GPRMC,013714,A,3408.6360,N,11812.0716,W,0.0,88.1,231105,13.5,E,D*09..","info-hex":"244750524d432c3031333731342c412c333430382e363336302c4e2c31313831322e303731362c572c302e302c38382e312c3233313130352c31332e352c452c442a30390d0a"}
```

## Build Requirements

### Mac OS X

* Xcode 5.1.1
* Qt 5.3

### Windows

* Visual Studio 2013
* Qt 5.3
* GLEW 1.10.0

## License

Copyright (C) 2014  h2so5

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
