#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#include "amqp_object_store.h"

void *amqp_object_store_get_valid_object(const zval *zobject TSRMLS_DC)
{
	zend_object_handle handle;
	zend_object_store_bucket bucket;

	handle = Z_OBJ_HANDLE_P(zobject);
	bucket = EG(objects_store).object_buckets[handle];

	return bucket.valid ? bucket.bucket.obj.object : 0;
}
