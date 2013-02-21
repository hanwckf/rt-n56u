/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2005, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file unit.c */

/**
 * \addtogroup PedUnit
 *
 * \brief The PedUnit module provides a standard mechanism for describing
 * and parsing locations within devices in human-friendly plain text.
 *
 * Internally, libparted uses PedSector (which is typedef'ed to be long long
 * in <parted/device.h>) to describe device locations such as the start and
 * end of partitions.  However, sector numbers are often long and unintuitive.
 * For example, my extended partition starts at sector 208845.  PedUnit allows
 * this location to be represented in more intutitive ways, including "106Mb",
 * "0Gb" and "0%", as well as "208845s".  PedUnit aims to provide facilities
 * to provide a consistent system for describing device locations all
 * throughout libparted.
 *
 * PedUnit provides two basic services: converting a PedSector into a text
 * representation, and parsing a text representation into a PedSector.
 * PedUnit currently supports these units:
 *
 * 	sectors, bytes, kilobytes, megabytes, gigabytes, terabytes, compact,
 * 	cylinder and percent.
 *
 * PedUnit has a global variable that contains the default unit for all
 * conversions.
 *
 * @{
 */




#include <config.h>
#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/unit.h>

#include <ctype.h>
#include <stdio.h>
#include <float.h>

#define N_(String) String
#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */


static PedUnit default_unit = PED_UNIT_COMPACT;
static const char* unit_names[] = {
	"s",
	"B",
	"kB",
	"MB",
	"GB",
	"TB",
	"compact",
	"cyl",
	"chs",
	"%",
	"kiB",
	"MiB",
	"GiB",
	"TiB"
};


/**
 * \brief Set the default \p unit used by subsequent calls to the PedUnit API.
 *
 * In particular, this affects how locations inside error messages
 * (exceptions) are displayed.
 */
void
ped_unit_set_default (PedUnit unit)
{
	default_unit = unit;
}


/**
 * \brief Get the current default unit.
 */
PedUnit _GL_ATTRIBUTE_PURE
ped_unit_get_default ()
{
	return default_unit;
}

/**
 * Get the byte size of a given \p unit.
 */
long long
ped_unit_get_size (const PedDevice* dev, PedUnit unit)
{
	PedSector cyl_size = dev->bios_geom.heads * dev->bios_geom.sectors;

	switch (unit) {
		case PED_UNIT_SECTOR:	return dev->sector_size;
		case PED_UNIT_BYTE:	return 1;
		case PED_UNIT_KILOBYTE:	return PED_KILOBYTE_SIZE;
		case PED_UNIT_MEGABYTE:	return PED_MEGABYTE_SIZE;
		case PED_UNIT_GIGABYTE:	return PED_GIGABYTE_SIZE;
		case PED_UNIT_TERABYTE:	return PED_TERABYTE_SIZE;
		case PED_UNIT_KIBIBYTE:	return PED_KIBIBYTE_SIZE;
		case PED_UNIT_MEBIBYTE:	return PED_MEBIBYTE_SIZE;
		case PED_UNIT_GIBIBYTE:	return PED_GIBIBYTE_SIZE;
		case PED_UNIT_TEBIBYTE:	return PED_TEBIBYTE_SIZE;
		case PED_UNIT_CYLINDER:	return cyl_size * dev->sector_size;
		case PED_UNIT_CHS:	return dev->sector_size;

		case PED_UNIT_PERCENT:
			return dev->length * dev->sector_size / 100;

		case PED_UNIT_COMPACT:
			ped_exception_throw (
				PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("Cannot get unit size for special unit "
				  "'COMPACT'."));
			return 0;
	}

	/* never reached */
	PED_ASSERT(0);
	return 0;
}

/**
 * Get a textual (non-internationalized) representation of a \p unit.
 *
 * For example, the textual representation of PED_UNIT_SECTOR is "s".
 */
const char*
ped_unit_get_name (PedUnit unit)
{
	return unit_names[unit];
}

/**
 * Get a unit based on its textual representation: \p unit_name.
 *
 * For example, ped_unit_get_by_name("Mb") returns PED_UNIT_MEGABYTE.
 */
PedUnit
ped_unit_get_by_name (const char* unit_name)
{
	PedUnit unit;
	for (unit = PED_UNIT_FIRST; unit <= PED_UNIT_LAST; unit++) {
		if (!strcasecmp (unit_names[unit], unit_name))
			return unit;
	}
	return -1;
}

static char*
ped_strdup (const char *str)
{
	char *result;
	result = ped_malloc (strlen (str) + 1);
	if (!result)
		return NULL;
	strcpy (result, str);
	return result;
}

/**
 * \brief Get a string that describes the location of the \p byte on
 * device \p dev.
 *
 * The string is described with the desired \p unit.
 * The returned string must be freed with free().
 */
char*
ped_unit_format_custom_byte (const PedDevice* dev, PedSector byte, PedUnit unit)
{
	char buf[100];
	PedSector sector = byte / dev->sector_size;
	double d, w;
	int p;

	PED_ASSERT (dev != NULL);

	/* CHS has a special comma-separated format. */
	if (unit == PED_UNIT_CHS) {
		const PedCHSGeometry *chs = &dev->bios_geom;
		snprintf (buf, 100, "%lld,%lld,%lld",
			  sector / chs->sectors / chs->heads,
			  (sector / chs->sectors) % chs->heads,
			  sector % chs->sectors);
		return ped_strdup (buf);
	}

	/* Cylinders, sectors and bytes should be rounded down... */
	if (unit == PED_UNIT_CYLINDER
	    || unit == PED_UNIT_SECTOR
	    || unit == PED_UNIT_BYTE) {
		snprintf (buf, 100, "%lld%s",
			  byte / ped_unit_get_size (dev, unit),
			  ped_unit_get_name (unit));
		return ped_strdup (buf);
	}

        if (unit == PED_UNIT_COMPACT) {
                if (byte >= 10LL * PED_TERABYTE_SIZE)
                        unit = PED_UNIT_TERABYTE;
                else if (byte >= 10LL * PED_GIGABYTE_SIZE)
                        unit = PED_UNIT_GIGABYTE;
                else if (byte >= 10LL * PED_MEGABYTE_SIZE)
                        unit = PED_UNIT_MEGABYTE;
                else if (byte >= 10LL * PED_KILOBYTE_SIZE)
                        unit = PED_UNIT_KILOBYTE;
                else
                        unit = PED_UNIT_BYTE;
	}

	/* IEEE754 says that 100.5 has to be rounded to 100 (by printf) */
	/* but 101.5 has to be rounded to 102... so we multiply by 1+E. */
	/* This just divide by 2 the natural IEEE754 extended precision */
	/* and won't cause any trouble before 1000 TB */
	d = ((double)byte / ped_unit_get_size (dev, unit))
	    * (1. + DBL_EPSILON);
	w = d + ( (d < 10. ) ? 0.005 :
		  (d < 100.) ? 0.05  :
			       0.5  );
	p = (w < 10. ) ? 2 :
	    (w < 100.) ? 1 :
			 0 ;

#ifdef __BEOS__
	snprintf (buf, 100, "%.*f%s", p, d, ped_unit_get_name(unit));
#else
	snprintf (buf, 100, "%1$.*2$f%3$s", d, p, ped_unit_get_name (unit));
#endif

	return ped_strdup (buf);
}

/**
 * \brief Get a string that describes the location of the \p byte on
 * device \p dev.
 *
 * The string is described with the default unit, which is set
 * by ped_unit_set_default().
 * The returned string must be freed with free().
 */
char*
ped_unit_format_byte (const PedDevice* dev, PedSector byte)
{
	PED_ASSERT (dev != NULL);
	return ped_unit_format_custom_byte (dev, byte, default_unit);
}

/**
 * \brief Get a string that describes the location \p sector on device \p dev.
 *
 * The string is described with the desired \p unit.
 * The returned string must be freed with free().
 */
char*
ped_unit_format_custom (const PedDevice* dev, PedSector sector, PedUnit unit)
{
	PED_ASSERT (dev != NULL);
	return ped_unit_format_custom_byte(dev, sector*dev->sector_size, unit);
}

/**
 * \brief Get a string that describes the location \p sector on device \p dev.
 *
 * The string is described with the default unit, which is set
 * by ped_unit_set_default().
 * The returned string must be freed with free().
 */
char*
ped_unit_format (const PedDevice* dev, PedSector sector)
{
	PED_ASSERT (dev != NULL);
	return ped_unit_format_custom_byte (dev, sector * dev->sector_size,
					    default_unit);
}

/**
 * If \p str contains a valid description of a location on \p dev,
 * then \p *sector is modified to describe the location and a geometry
 * is created in \p *range describing a 2 units large area centered on
 * \p *sector.  If the \p range as described here would be partially outside
 * the device \p dev, the geometry returned is the intersection between the
 * former and the whole	device geometry.  If no units are specified, then the
 * default unit is assumed.
 *
 * \return \c 1 if \p str is a valid location description, \c 0 otherwise
 */
int
ped_unit_parse (const char* str, const PedDevice* dev, PedSector *sector,
		PedGeometry** range)
{
	return ped_unit_parse_custom (str, dev, default_unit, sector, range);
}

/* Inefficiently removes all spaces from a string, in-place. */
static void
strip_string (char* str)
{
	int i;

	for (i = 0; str[i] != 0; i++) {
		if (isspace (str[i])) {
			int j;
			for (j = i + 1; str[j] != 0; j++)
				str[j - 1] = str[j];
		}
	}
}


/* Find non-number suffix.  Eg: find_suffix("32Mb") returns a pointer to
 * "Mb". */
static char* _GL_ATTRIBUTE_PURE
find_suffix (const char* str)
{
	while (str[0] != 0 && (isdigit (str[0]) || strchr(",.-", str[0])))
		str++;
	return (char *) str;
}

static void
remove_punct (char* str)
{
	int i = 0;

	for (i = 0; str[i]; i++) {
		if (ispunct (str[i]))
			str[i] = ' ';
	}
}

static int _GL_ATTRIBUTE_PURE
is_chs (const char* str)
{
	int punct_count = 0;
	int i = 0;

	for (i = 0; str[i]; i++)
		punct_count += ispunct (str[i]) != 0;
	return punct_count == 2;
}

static int
parse_chs (const char* str, const PedDevice* dev, PedSector* sector,
		PedGeometry** range)
{
	PedSector cyl_size = dev->bios_geom.heads * dev->bios_geom.sectors;
	PedCHSGeometry chs;

	char* copy = ped_strdup (str);
	if (!copy)
		return 0;
	strip_string (copy);
	remove_punct (copy);

	if (sscanf (copy, "%d %d %d",
		    &chs.cylinders, &chs.heads, &chs.sectors) != 3) {
		ped_exception_throw (
				PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("\"%s\" has invalid syntax for locations."),
				copy);
		goto error_free_copy;
	}

	if (chs.heads >= dev->bios_geom.heads) {
		ped_exception_throw (
				PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("The maximum head value is %d."),
				dev->bios_geom.heads - 1);
		goto error_free_copy;
	}
	if (chs.sectors >= dev->bios_geom.sectors) {
		ped_exception_throw (
				PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("The maximum sector value is %d."),
				dev->bios_geom.sectors - 1);
		goto error_free_copy;
	}

	*sector = 1LL * chs.cylinders * cyl_size
		+ chs.heads * dev->bios_geom.sectors
		+ chs.sectors;

	if (*sector >= dev->length) {
		ped_exception_throw (
				PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("The location %s is outside of the "
				  "device %s."),
				str, dev->path);
		goto error_free_copy;
	}
	if (range)
		*range = ped_geometry_new (dev, *sector, 1);
	free (copy);
	return !range || *range != NULL;

error_free_copy:
	free (copy);
	*sector = 0;
	if (range)
		*range = NULL;
	return 0;
}

static PedSector
clip (const PedDevice* dev, PedSector sector)
{
	if (sector < 0)
		return 0;
	if (sector > dev->length - 1)
		return dev->length - 1;
	return sector;
}

static PedGeometry*
geometry_from_centre_radius (const PedDevice* dev,
                             PedSector sector, PedSector radius)
{
	PedSector start = clip (dev, sector - radius);
	PedSector end = clip (dev, sector + radius);
	if (sector - end > radius || start - sector > radius)
		return NULL;
	return ped_geometry_new (dev, start, end - start + 1);
}

static PedUnit
parse_unit_suffix (const char* suffix, PedUnit suggested_unit)
{
	if (strlen (suffix) > 1 && tolower (suffix[1]) == 'i') {
		switch (tolower (suffix[0])) {
			case 'k': return PED_UNIT_KIBIBYTE;
			case 'm': return PED_UNIT_MEBIBYTE;
			case 'g': return PED_UNIT_GIBIBYTE;
			case 't': return PED_UNIT_TEBIBYTE;
		}
	} else if (strlen (suffix) > 0) {
		switch (tolower (suffix[0])) {
			case 's': return PED_UNIT_SECTOR;
			case 'b': return PED_UNIT_BYTE;
			case 'k': return PED_UNIT_KILOBYTE;
			case 'm': return PED_UNIT_MEGABYTE;
			case 'g': return PED_UNIT_GIGABYTE;
			case 't': return PED_UNIT_TERABYTE;
			case 'c': return PED_UNIT_CYLINDER;
			case '%': return PED_UNIT_PERCENT;
		}
	}

	if (suggested_unit == PED_UNIT_COMPACT) {
		if (default_unit == PED_UNIT_COMPACT)
			return PED_UNIT_MEGABYTE;
		else
			return default_unit;
	}

	return suggested_unit;
}

static bool
is_power_of_2 (long long n)
{
  return (n & (n - 1)) == 0;
}

/**
 * If \p str contains a valid description of a location on \p dev, then
 * \p *sector is modified to describe the location and a geometry is created
 * in \p *range describing a 2 units large area centered on \p *sector.  If the
 * \p range as described here would be partially outside the device \p dev, the
 * geometry returned is the intersection between the former and the whole
 * device geometry.  If no units are specified, then the default unit is
 * assumed.
 *
 * \throws PED_EXCEPTION_ERROR if \p str contains invalid description of a
 * location
 * \throws PED_EXCEPTION_ERROR if location described by \p str
 * is outside of the device \p dev->path
 *
 * \return \c 1 if \p str is a valid location description, \c 0 otherwise.
 */
int
ped_unit_parse_custom (const char* str, const PedDevice* dev, PedUnit unit,
		       PedSector* sector, PedGeometry** range)
{
	char*     copy;
	char*     suffix;
	double    num;
	long long unit_size;
	PedSector radius;

	if (is_chs (str))
		return parse_chs (str, dev, sector, range);

	copy = ped_strdup (str);
	if (!copy)
		goto error;
	strip_string (copy);

	suffix = find_suffix (copy);
	unit = parse_unit_suffix (suffix, unit);
	suffix[0] = 0;

	if (sscanf (copy, "%lf", &num) != 1) {
		ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Invalid number."));
		goto error_free_copy;
	}
        if (num > 0 && num < 1) {
            ped_exception_throw (
                    PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
                    _("Use a smaller unit instead of a value < 1"));
            goto error_free_copy;
        }

	unit_size = ped_unit_get_size (dev, unit);
	radius = (ped_div_round_up (unit_size, dev->sector_size) / 2) - 1;
	if (radius < 0)
		radius = 0;
	/* If the user specifies units in a power of 2, e.g., 4MiB, as in
	       parted -s -- $dev mklabel gpt mkpart P-NAME 4MiB -34s
	   do not use 4MiB as the range.  Rather, presume that they
	   are specifying precisely the starting or ending number,
	   and treat "4MiB" just as we would treat "4194304B".  */
	if (is_power_of_2 (unit_size))
		radius = 0;

	*sector = num * unit_size / dev->sector_size;
	/* negative numbers count from the end */
	if (copy[0] == '-')
		*sector += dev->length;
	if (range) {
		*range = geometry_from_centre_radius (dev, *sector, radius);
		if (!*range) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("The location %s is outside of the "
				  "device %s."),
				str, dev->path);
			goto error_free_copy;
		}
	}
	*sector = clip (dev, *sector);

	free (copy);
	return 1;

error_free_copy:
	free (copy);
error:
	*sector = 0;
	if (range)
		*range = NULL;
	return 0;
}


/** @} */
