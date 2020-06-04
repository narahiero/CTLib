# CT Lib

[![License](https://img.shields.io/github/license/narahiero/CTLib)](https://github.com/narahiero/CTLib/blob/master/License.txt)
[![Latest release](https://img.shields.io/github/v/release/narahiero/CTLib?include_prereleases)](https://github.com/narahiero/CTLib/releases)

CT Lib is a C++ library that can read and write Nintendo file formats that are
used by Mario Kart Wii. It is licensed under GPLv3+.

## Format Support

| **Format**      | **Status**         |
|:--------------- |:------------------:|
| Yaz0/Yaz1       | Fully Supported    |
| U8              | Fully Supported    |
| BRRES           | Basic Support      |
| KCL             | Mostly Supported   |
| KMP             | Mostly Supported   |

### BRRES Subfiles Support

| **Subfile**     | **Status**         |
|:--------------- |:------------------:|
| MDL0            | Basic Support      |
| TEX0            | Mostly Supported\* |

\* The image formats C4, C8, and C14X2 are not supported.
