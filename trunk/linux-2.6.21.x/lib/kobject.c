/*
 * kobject.c - library routines for handling generic kernel objects
 *
 * Copyright (c) 2002-2003 Patrick Mochel <mochel@osdl.org>
 *
 * This file is released under the GPLv2.
 *
 *
 * Please see the file Documentation/kobject.txt for critical information
 * about using the kobject interface.
 */

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/slab.h>

/**
 *	populate_dir - populate directory with attributes.
 *	@kobj:	object we're working on.
 *
 *	Most subsystems have a set of default attributes that 
 *	are associated with an object that registers with them.
 *	This is a helper called during object registration that 
 *	loops through the default attributes of the subsystem 
 *	and creates attributes files for them in sysfs.
 *
 */

static int populate_dir(struct kobject * kobj)
{
	struct kobj_type * t = get_ktype(kobj);
	struct attribute * attr;
	int error = 0;
	int i;
	
	if (t && t->default_attrs) {
		for (i = 0; (attr = t->default_attrs[i]) != NULL; i++) {
			if ((error = sysfs_create_file(kobj,attr)))
				break;
		}
	}
	return error;
}

static int create_dir(struct kobject * kobj, struct dentry *shadow_parent)
{
	int error = 0;
	if (kobject_name(kobj)) {
		error = sysfs_create_dir(kobj, shadow_parent);
		if (!error) {
			if ((error = populate_dir(kobj)))
				sysfs_remove_dir(kobj);
		}
	}
	return error;
}

static inline struct kobject * to_kobj(struct list_head * entry)
{
	return container_of(entry,struct kobject,entry);
}

static int get_kobj_path_length(struct kobject *kobj)
{
	int length = 1;
	struct kobject * parent = kobj;

	/* walk up the ancestors until we hit the one pointing to the 
	 * root.
	 * Add 1 to strlen for leading '/' of each level.
	 */
	do {
		if (kobject_name(parent) == NULL)
			return 0;
		length += strlen(kobject_name(parent)) + 1;
		parent = parent->parent;
	} while (parent);
	return length;
}

static void fill_kobj_path(struct kobject *kobj, char *path, int length)
{
	struct kobject * parent;

	--length;
	for (parent = kobj; parent; parent = parent->parent) {
		int cur = strlen(kobject_name(parent));
		/* back up enough to print this name with '/' */
		length -= cur;
		strncpy (path + length, kobject_name(parent), cur);
		*(path + --length) = '/';
	}

	pr_debug("%s: path = '%s'\n",__FUNCTION__,path);
}

/**
 * kobject_get_path - generate and return the path associated with a given kobj and kset pair.
 *
 * @kobj:	kobject in question, with which to build the path
 * @gfp_mask:	the allocation type used to allocate the path
 *
 * The result must be freed by the caller with kfree().
 */
char *kobject_get_path(struct kobject *kobj, gfp_t gfp_mask)
{
	char *path;
	int len;

	len = get_kobj_path_length(kobj);
	if (len == 0)
		return NULL;
	path = kzalloc(len, gfp_mask);
	if (!path)
		return NULL;
	fill_kobj_path(kobj, path, len);

	return path;
}
EXPORT_SYMBOL_GPL(kobject_get_path);

/**
 *	kobject_init - initialize object.
 *	@kobj:	object in question.
 */
void kobject_init(struct kobject * kobj)
{
	if (!kobj)
		return;
	kref_init(&kobj->kref);
	INIT_LIST_HEAD(&kobj->entry);
	init_waitqueue_head(&kobj->poll);
	kobj->kset = kset_get(kobj->kset);
}


/**
 *	unlink - remove kobject from kset list.
 *	@kobj:	kobject.
 *
 *	Remove the kobject from the kset list and decrement
 *	its parent's refcount.
 *	This is separated out, so we can use it in both 
 *	kobject_del() and kobject_add() on error.
 */

static void unlink(struct kobject * kobj)
{
	if (kobj->kset) {
		spin_lock(&kobj->kset->list_lock);
		list_del_init(&kobj->entry);
		spin_unlock(&kobj->kset->list_lock);
	}
	kobject_put(kobj);
}

/**
 *	kobject_add - add an object to the hierarchy.
 *	@kobj:	object.
 *	@shadow_parent: sysfs directory to add to.
 */

int kobject_shadow_add(struct kobject * kobj, struct dentry *shadow_parent)
{
	int error = 0;
	struct kobject * parent;

	if (!(kobj = kobject_get(kobj)))
		return -ENOENT;
	if (!kobj->k_name)
		kobj->k_name = kobj->name;
	if (!*kobj->k_name) {
		pr_debug("kobject attempted to be registered with no name!\n");
		WARN_ON(1);
		kobject_put(kobj);
		return -EINVAL;
	}
	parent = kobject_get(kobj->parent);

	pr_debug("kobject %s: registering. parent: %s, set: %s\n",
		 kobject_name(kobj), parent ? kobject_name(parent) : "<NULL>", 
		 kobj->kset ? kobj->kset->kobj.name : "<NULL>" );

	if (kobj->kset) {
		spin_lock(&kobj->kset->list_lock);

		if (!parent)
			parent = kobject_get(&kobj->kset->kobj);

		list_add_tail(&kobj->entry,&kobj->kset->list);
		spin_unlock(&kobj->kset->list_lock);
	}
	kobj->parent = parent;

	error = create_dir(kobj, shadow_parent);
	if (error) {
		/* unlink does the kobject_put() for us */
		unlink(kobj);
		kobject_put(parent);

		/* be noisy on error issues */
		if (error == -EEXIST)
			printk("kobject_add failed for %s with -EEXIST, "
			       "don't try to register things with the "
			       "same name in the same directory.\n",
			       kobject_name(kobj));
		else
			printk("kobject_add failed for %s (%d)\n",
			       kobject_name(kobj), error);
		 dump_stack();
	}

	return error;
}

/**
 *	kobject_add - add an object to the hierarchy.
 *	@kobj:	object.
 */
int kobject_add(struct kobject * kobj)
{
	return kobject_shadow_add(kobj, NULL);
}


/**
 *	kobject_register - initialize and add an object.
 *	@kobj:	object in question.
 */

int kobject_register(struct kobject * kobj)
{
	int error = -EINVAL;
	if (kobj) {
		kobject_init(kobj);
		error = kobject_add(kobj);
		if (!error)
			kobject_uevent(kobj, KOBJ_ADD);
	}
	return error;
}

/**
 * kobject_set_name_vargs - Set the name of an kobject
 * @kobj: struct kobject to set the name of
 * @fmt: format string used to build the name
 * @vargs: vargs to format the string.
 */
static int kobject_set_name_vargs(struct kobject *kobj, const char *fmt,
				  va_list vargs)
{
	va_list aq;
	char *name;

	va_copy(aq, vargs);
	name = kvasprintf(GFP_KERNEL, fmt, vargs);
	va_end(aq);

	if (!name)
		return -ENOMEM;

	/* Free the old name, if necessary. */
	kfree(kobj->k_name);

	/* Now, set the new name */
	kobj->k_name = name;

	return 0;
}

/**
 * kobject_set_name - Set the name of a kobject
 * @kobj: struct kobject to set the name of
 * @fmt: format string used to build the name
 *
 * This sets the name of the kobject.  If you have already added the
 * kobject to the system, you must call kobject_rename() in order to
 * change the name of the kobject.
 */
int kobject_set_name(struct kobject *kobj, const char *fmt, ...)
{
	va_list args;
	int retval;

 	va_start(args, fmt);
	retval = kobject_set_name_vargs(kobj, fmt, args);
	va_end(args);
	return retval;
}
EXPORT_SYMBOL(kobject_set_name);


/**
 * kobject_init_ng - initialize a kobject structure
 * @kobj: pointer to the kobject to initialize
 * @ktype: pointer to the ktype for this kobject.
 *
 * This function will properly initialize a kobject such that it can then
 * be passed to the kobject_add() call.
 *
 * After this function is called, the kobject MUST be cleaned up by a call
 * to kobject_put(), not by a call to kfree directly to ensure that all of
 * the memory is cleaned up properly.
 */
void kobject_init_ng(struct kobject *kobj, struct kobj_type *ktype)
{
	char *err_str;

	if (!kobj) {
		err_str = "invalid kobject pointer!";
		goto error;
	}
	if (!ktype) {
		err_str = "must have a ktype to be initialized properly!\n";
		goto error;
	}
	if (atomic_read(&kobj->kref.refcount)) {
		/* do not error out as sometimes we can recover */
		printk(KERN_ERR "kobject: reference count is already set, "
		       "something is seriously wrong.\n");
		dump_stack();
	}

	kref_init(&kobj->kref);
	INIT_LIST_HEAD(&kobj->entry);
	kobj->ktype = ktype;
	return;

error:
	printk(KERN_ERR "kobject: %s\n", err_str);
	dump_stack();
}
EXPORT_SYMBOL(kobject_init_ng);

static int kobject_add_varg(struct kobject *kobj, struct kobject *parent,
			    const char *fmt, va_list vargs)
{
	va_list aq;
	int retval;

	va_copy(aq, vargs);
	retval = kobject_set_name_vargs(kobj, fmt, aq);
	va_end(aq);
	if (retval) {
		printk(KERN_ERR "kobject: can not set name properly!\n");
		return retval;
	}
	kobj->parent = parent;
	return kobject_add(kobj);
}

/**
 * kobject_add_ng - the main kobject add function
 * @kobj: the kobject to add
 * @parent: pointer to the parent of the kobject.
 * @fmt: format to name the kobject with.
 *
 * The kobject name is set and added to the kobject hierarchy in this
 * function.
 *
 * If @parent is set, then the parent of the @kobj will be set to it.
 * If @parent is NULL, then the parent of the @kobj will be set to the
 * kobject associted with the kset assigned to this kobject.  If no kset
 * is assigned to the kobject, then the kobject will be located in the
 * root of the sysfs tree.
 *
 * If this function returns an error, kobject_put() must be called to
 * properly clean up the memory associated with the object.
 *
 * If the function is successful, the only way to properly clean up the
 * memory is with a call to kobject_del(), in which case, a call to
 * kobject_put() is not necessary (kobject_del() does the final
 * kobject_put() to call the release function in the ktype's release
 * pointer.)
 *
 * Under no instance should the kobject that is passed to this function
 * be directly freed with a call to kfree(), that can leak memory.
 *
 * Note, no uevent will be created with this call, the caller should set
 * up all of the necessary sysfs files for the object and then call
 * kobject_uevent() with the UEVENT_ADD parameter to ensure that
 * userspace is properly notified of this kobject's creation.
 */
int kobject_add_ng(struct kobject *kobj, struct kobject *parent,
		   const char *fmt, ...)
{
	va_list args;
	int retval;

	if (!kobj)
		return -EINVAL;

	va_start(args, fmt);
	retval = kobject_add_varg(kobj, parent, fmt, args);
	va_end(args);

	return retval;
}
EXPORT_SYMBOL(kobject_add_ng);

/**
 *	kobject_rename - change the name of an object
 *	@kobj:	object in question.
 *	@new_name: object's new name
 */

int kobject_rename(struct kobject * kobj, const char *new_name)
{
	int error = 0;
	const char *devpath = NULL;
	char *devpath_string = NULL;
	char *envp[2];

	kobj = kobject_get(kobj);
	if (!kobj)
		return -EINVAL;
	if (!kobj->parent)
		return -EINVAL;

	devpath = kobject_get_path(kobj, GFP_KERNEL);
	if (!devpath) {
		error = -ENOMEM;
		goto out;
	}
	devpath_string = kmalloc(strlen(devpath) + 15, GFP_KERNEL);
	if (!devpath_string) {
		error = -ENOMEM;
		goto out;
	}
	sprintf(devpath_string, "DEVPATH_OLD=%s", devpath);
	envp[0] = devpath_string;
	envp[1] = NULL;
	/* Note : if we want to send the new name alone, not the full path,
	 * we could probably use kobject_name(kobj); */

	error = sysfs_rename_dir(kobj, kobj->parent->dentry, new_name);

	/* This function is mostly/only used for network interface.
	 * Some hotplug package track interfaces by their name and
	 * therefore want to know when the name is changed by the user. */
	if (!error)
		kobject_uevent_env(kobj, KOBJ_MOVE, envp);

out:
	kfree(devpath_string);
	kfree(devpath);
	kobject_put(kobj);

	return error;
}

/**
 *	kobject_rename - change the name of an object
 *	@kobj:	object in question.
 *	@new_parent: object's new parent
 *	@new_name: object's new name
 */

int kobject_shadow_rename(struct kobject * kobj, struct dentry *new_parent,
			  const char *new_name)
{
	int error = 0;

	kobj = kobject_get(kobj);
	if (!kobj)
		return -EINVAL;
	error = sysfs_rename_dir(kobj, new_parent, new_name);
	kobject_put(kobj);

	return error;
}

static void dir_release(struct kobject *kobj)
{
	kfree(kobj);
}

static struct kobj_type dir_ktype = {
	.release	= dir_release,
	.sysfs_ops	= NULL,
	.default_attrs	= NULL,
};

/**
 *	kobject_add_dir - add sub directory of object.
 *	@parent:	object in which a directory is created.
 *	@name:	directory name.
 *
 *	Add a plain directory object as child of given object.
 */
struct kobject *kobject_add_dir(struct kobject *parent, const char *name)
{
	struct kobject *k;

	if (!parent)
		return NULL;

	k = kzalloc(sizeof(*k), GFP_KERNEL);
	if (!k)
		return NULL;

	k->parent = parent;
	k->ktype = &dir_ktype;
	kobject_set_name(k, name);
	kobject_register(k);

	return k;
}
EXPORT_SYMBOL_GPL(kobject_add_dir);

/**
 *	kobject_move - move object to another parent
 *	@kobj:	object in question.
 *	@new_parent: object's new parent (can be NULL)
 */

int kobject_move(struct kobject *kobj, struct kobject *new_parent)
{
	int error;
	struct kobject *old_parent;
	const char *devpath = NULL;
	char *devpath_string = NULL;
	char *envp[2];

	kobj = kobject_get(kobj);
	if (!kobj)
		return -EINVAL;
	new_parent = kobject_get(new_parent);
	if (!new_parent) {
		if (kobj->kset)
			new_parent = kobject_get(&kobj->kset->kobj);
	}
	/* old object path */
	devpath = kobject_get_path(kobj, GFP_KERNEL);
	if (!devpath) {
		error = -ENOMEM;
		goto out;
	}
	devpath_string = kmalloc(strlen(devpath) + 15, GFP_KERNEL);
	if (!devpath_string) {
		error = -ENOMEM;
		goto out;
	}
	sprintf(devpath_string, "DEVPATH_OLD=%s", devpath);
	envp[0] = devpath_string;
	envp[1] = NULL;
	error = sysfs_move_dir(kobj, new_parent);
	if (error)
		goto out;
	old_parent = kobj->parent;
	kobj->parent = new_parent;
	new_parent = NULL;
	kobject_put(old_parent);
	kobject_uevent_env(kobj, KOBJ_MOVE, envp);
out:
	kobject_put(new_parent);
	kobject_put(kobj);
	kfree(devpath_string);
	kfree(devpath);
	return error;
}

/**
 *	kobject_del - unlink kobject from hierarchy.
 * 	@kobj:	object.
 */

void kobject_del(struct kobject * kobj)
{
	if (!kobj)
		return;
	sysfs_remove_dir(kobj);
	unlink(kobj);
}

/**
 *	kobject_unregister - remove object from hierarchy and decrement refcount.
 *	@kobj:	object going away.
 */

void kobject_unregister(struct kobject * kobj)
{
	if (!kobj)
		return;
	pr_debug("kobject %s: unregistering\n",kobject_name(kobj));
	kobject_uevent(kobj, KOBJ_REMOVE);
	kobject_del(kobj);
	kobject_put(kobj);
}

/**
 *	kobject_get - increment refcount for object.
 *	@kobj:	object.
 */

struct kobject * kobject_get(struct kobject * kobj)
{
	if (kobj)
		kref_get(&kobj->kref);
	return kobj;
}

/**
 *	kobject_cleanup - free kobject resources. 
 *	@kobj:	object.
 */

void kobject_cleanup(struct kobject * kobj)
{
	struct kobj_type * t = get_ktype(kobj);
	struct kset * s = kobj->kset;
	struct kobject * parent = kobj->parent;

	pr_debug("kobject %s: cleaning up\n",kobject_name(kobj));
	if (kobj->k_name != kobj->name)
		kfree(kobj->k_name);
	kobj->k_name = NULL;
	if (t && t->release)
		t->release(kobj);
	if (s)
		kset_put(s);
	kobject_put(parent);
}

static void kobject_release(struct kref *kref)
{
	kobject_cleanup(container_of(kref, struct kobject, kref));
}

/**
 *	kobject_put - decrement refcount for object.
 *	@kobj:	object.
 *
 *	Decrement the refcount, and if 0, call kobject_cleanup().
 */
void kobject_put(struct kobject * kobj)
{
	if (kobj)
		kref_put(&kobj->kref, kobject_release);
}

static void dynamic_kobj_release(struct kobject *kobj)
{
	pr_debug("%s: freeing %s\n", __FUNCTION__, kobject_name(kobj));
	kfree(kobj);
}

static struct kobj_type dynamic_kobj_ktype = {
	.release = dynamic_kobj_release,
};

/*
 * kobject_create - create a struct kobject dynamically
 *
 * This function creates a kobject structure dynamically and sets it up
 * to be a "dynamic" kobject with a default release function set up.
 *
 * If the kobject was not able to be created, NULL will be returned.
 */
static struct kobject *kobject_create(void)
{
	struct kobject *kobj;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (!kobj)
		return NULL;

	kobject_init_ng(kobj, &dynamic_kobj_ktype);
	return kobj;
}

/**
 * kobject_create_and_add - create a struct kobject dynamically and register it with sysfs
 *
 * @name: the name for the kset
 * @parent: the parent kobject of this kobject, if any.
 *
 * This function creates a kset structure dynamically and registers it
 * with sysfs.  When you are finished with this structure, call
 * kobject_unregister() and the structure will be dynamically freed when
 * it is no longer being used.
 *
 * If the kobject was not able to be created, NULL will be returned.
 */
struct kobject *kobject_create_and_add(const char *name, struct kobject *parent)
{
	struct kobject *kobj;
	int retval;

	kobj = kobject_create();
	if (!kobj)
		return NULL;

	retval = kobject_add_ng(kobj, parent, "%s", name);
	if (retval) {
		printk(KERN_WARNING "%s: kobject_add error: %d\n",
		       __FUNCTION__, retval);
		kobject_put(kobj);
		kobj = NULL;
	}
	return kobj;
}
EXPORT_SYMBOL_GPL(kobject_create_and_add);

/**
 *	kobject_kset_add_dir - add sub directory of object.
 *	@kset:		kset the directory is belongs to.
 *	@parent:	object in which a directory is created.
 *	@name:	directory name.
 *
 *	Add a plain directory object as child of given object.
 */
struct kobject *kobject_kset_add_dir(struct kset *kset,
				     struct kobject *parent, const char *name)
{
	struct kobject *k;
	int ret;

	k = kobject_create();
	if (!k)
		return NULL;

	k->kset = kset;
	ret = kobject_add_ng(k, parent, "%s", name);
	if (ret < 0) {
		printk(KERN_WARNING "%s: kobject_add error: %d\n",
			__func__, ret);
		kobject_put(k);
		k = NULL;
	}

	return k;
}

/**
 *	kset_init - initialize a kset for use
 *	@k:	kset 
 */

void kset_init(struct kset * k)
{
	kobject_init(&k->kobj);
	INIT_LIST_HEAD(&k->list);
	spin_lock_init(&k->list_lock);
}


/**
 *	kset_add - add a kset object to the hierarchy.
 *	@k:	kset.
 */

int kset_add(struct kset * k)
{
	return kobject_add(&k->kobj);
}


/**
 *	kset_register - initialize and add a kset.
 *	@k:	kset.
 */

int kset_register(struct kset * k)
{
	int err;

	if (!k)
		return -EINVAL;

	kset_init(k);
	err = kset_add(k);
	if (err)
		return err;
	kobject_uevent(&k->kobj, KOBJ_ADD);
	return 0;
}


/**
 *	kset_unregister - remove a kset.
 *	@k:	kset.
 */

void kset_unregister(struct kset * k)
{
	if (!k)
		return;
	kobject_unregister(&k->kobj);
}


/**
 *	kset_find_obj - search for object in kset.
 *	@kset:	kset we're looking in.
 *	@name:	object's name.
 *
 *	Lock kset via @kset->subsys, and iterate over @kset->list,
 *	looking for a matching kobject. If matching object is found
 *	take a reference and return the object.
 */

struct kobject * kset_find_obj(struct kset * kset, const char * name)
{
	struct list_head * entry;
	struct kobject * ret = NULL;

	spin_lock(&kset->list_lock);
	list_for_each(entry,&kset->list) {
		struct kobject * k = to_kobj(entry);
		if (kobject_name(k) && !strcmp(kobject_name(k),name)) {
			ret = kobject_get(k);
			break;
		}
	}
	spin_unlock(&kset->list_lock);
	return ret;
}

void subsystem_init(struct kset *s)
{
	kset_init(s);
}

int subsystem_register(struct kset *s)
{
	return kset_register(s);
}

void subsystem_unregister(struct kset *s)
{
	kset_unregister(s);
}

/**
 *	subsystem_create_file - export sysfs attribute file.
 *	@s:	subsystem.
 *	@a:	subsystem attribute descriptor.
 */

int subsys_create_file(struct kset *s, struct subsys_attribute *a)
{
	int error = 0;

	if (!s || !a)
		return -EINVAL;

	if (subsys_get(s)) {
		error = sysfs_create_file(&s->kobj, &a->attr);
		subsys_put(s);
	}
	return error;
}

EXPORT_SYMBOL(kobject_init);
EXPORT_SYMBOL(kobject_register);
EXPORT_SYMBOL(kobject_unregister);
EXPORT_SYMBOL(kobject_get);
EXPORT_SYMBOL(kobject_put);
EXPORT_SYMBOL(kobject_add);
EXPORT_SYMBOL(kobject_del);

EXPORT_SYMBOL(kset_register);
EXPORT_SYMBOL(kset_unregister);

EXPORT_SYMBOL(subsystem_register);
EXPORT_SYMBOL(subsystem_unregister);
EXPORT_SYMBOL(subsys_create_file);
