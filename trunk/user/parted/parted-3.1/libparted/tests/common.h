#include <parted/parted.h>

/* Create an empty disk image
 *
 * filename: file (with full path) where to write the disk image
 *     size: size of disk image (megabytes)
 */
char* _create_disk (const off_t size);

/* Create a disk label
 *
 *  dev: device to use when creating the label
 * type: label type
 */
PedDisk* _create_disk_label (PedDevice* dev, PedDiskType* type);

/* Return if a disk label is implemented
 *
 * label: disk label name
 */
int _implemented_disk_label (const char* label) _GL_ATTRIBUTE_PURE;

/* Test specific exception handler
 *
 */
PedExceptionOption _test_exception_handler (PedException* e);
