/* Copyright (C) 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 * Tree search generalized from Knuth (6.2.2) Algorithm T just like
 * the AT&T man page says.
 *
 * The node_t structure is for internal use only, lint doesn't grok it.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 */
/*LINTLIBRARY*/

#include <search.h>
#include <stdlib.h>

/* This routine is not very bad. It makes many assumptions about
 * the compiler. It assumpts that the first field in node must be
 * the "key" field, which points to the datum. It is a very trick
 * stuff. H.J.
 */

typedef struct node_t
{
    void	*key;
    struct node_t *left, *right;
} node;

#ifdef L_tsearch
/* find or insert datum into search tree.
char 	*key;			 key to be located
register node	**rootp;	 address of tree root
int	(*compar)();		 ordering function
*/

void *tsearch(__const void *key, void **vrootp, __compar_fn_t compar)
{
    register node *q;
    register node **rootp = (node **) vrootp;

    if (rootp == (struct node_t **)0)
	return ((struct node_t *)0);
    while (*rootp != (struct node_t *)0)	/* Knuth's T1: */
    {
	int r;

	if ((r = (*compar)(key, (*rootp)->key)) == 0)	/* T2: */
	    return (*rootp);		/* we found it! */
	rootp = (r < 0) ?
	    &(*rootp)->left :		/* T3: follow left branch */
	    &(*rootp)->right;		/* T4: follow right branch */
    }
    q = (node *) malloc(sizeof(node));	/* T5: key not found */
    if (q != (struct node_t *)0)	/* make new node */
    {
	*rootp = q;			/* link new node to old */
	q->key = (void *)key;			/* initialize new node */
	q->left = q->right = (struct node_t *)0;
    }
    return (q);
}
libc_hidden_def(tsearch)
#endif

#ifdef L_tfind
void *tfind(__const void *key, void * __const *vrootp, __compar_fn_t compar)
{
    register node **rootp = (node **) vrootp;

    if (rootp == (struct node_t **)0)
	return ((struct node_t *)0);
    while (*rootp != (struct node_t *)0)	/* Knuth's T1: */
    {
	int r;

	if ((r = (*compar)(key, (*rootp)->key)) == 0)	/* T2: */
	    return (*rootp);		/* we found it! */
	rootp = (r < 0) ?
	    &(*rootp)->left :		/* T3: follow left branch */
	    &(*rootp)->right;		/* T4: follow right branch */
    }
    return NULL;
}
libc_hidden_def(tfind)
#endif

#ifdef L_tdelete
/* delete node with given key
char	*key;			key to be deleted
register node	**rootp;	address of the root of tree
int	(*compar)();		comparison function
*/
void *tdelete(__const void *key, void ** vrootp, __compar_fn_t compar)
{
    node *p;
    register node *q;
    register node *r;
    int cmp;
    register node **rootp = (node **) vrootp;

    if (rootp == (struct node_t **)0 || (p = *rootp) == (struct node_t *)0)
	return ((struct node_t *)0);
    while ((cmp = (*compar)(key, (*rootp)->key)) != 0)
    {
	p = *rootp;
	rootp = (cmp < 0) ?
	    &(*rootp)->left :		/* follow left branch */
	    &(*rootp)->right;		/* follow right branch */
	if (*rootp == (struct node_t *)0)
	    return ((struct node_t *)0);	/* key not found */
    }
    r = (*rootp)->right;			/* D1: */
    if ((q = (*rootp)->left) == (struct node_t *)0)	/* Left (struct node_t *)0? */
	q = r;
    else if (r != (struct node_t *)0)		/* Right link is null? */
    {
	if (r->left == (struct node_t *)0)	/* D2: Find successor */
	{
	    r->left = q;
	    q = r;
	}
	else
	{			/* D3: Find (struct node_t *)0 link */
	    for (q = r->left; q->left != (struct node_t *)0; q = r->left)
		r = q;
	    r->left = q->right;
	    q->left = (*rootp)->left;
	    q->right = (*rootp)->right;
	}
    }
    free((struct node_t *) *rootp);	/* D4: Free node */
    *rootp = q;				/* link parent to new node */
    return(p);
}
#endif

#ifdef L_twalk
/* Walk the nodes of a tree
register node	*root;		Root of the tree to be walked
register void	(*action)();	Function to be called at each node
register int	level;
*/
static void trecurse(__const void *vroot, __action_fn_t action, int level)
{
    register node *root = (node *) vroot;

    if (root->left == (struct node_t *)0 && root->right == (struct node_t *)0)
	(*action)(root, leaf, level);
    else
    {
	(*action)(root, preorder, level);
	if (root->left != (struct node_t *)0)
	    trecurse(root->left, action, level + 1);
	(*action)(root, postorder, level);
	if (root->right != (struct node_t *)0)
	    trecurse(root->right, action, level + 1);
	(*action)(root, endorder, level);
    }
}

/* void twalk(root, action)		Walk the nodes of a tree
node	*root;			Root of the tree to be walked
void	(*action)();		Function to be called at each node
PTR
*/
void twalk(__const void *vroot, __action_fn_t action)
{
    register __const node *root = (node *) vroot;

    if (root != (node *)0 && action != (__action_fn_t) 0)
	trecurse(root, action, 0);
}
#endif

#ifdef __USE_GNU
#ifdef L_tdestroy
/* The standardized functions miss an important functionality: the
   tree cannot be removed easily.  We provide a function to do this.  */
static void
internal_function
tdestroy_recurse (node *root, __free_fn_t freefct)
{
    if (root->left != NULL)
	tdestroy_recurse (root->left, freefct);
    if (root->right != NULL)
	tdestroy_recurse (root->right, freefct);
    (*freefct) ((void *) root->key);
    /* Free the node itself.  */
    free (root);
}

void tdestroy (void *vroot, __free_fn_t freefct)
{
    node *root = (node *) vroot;
    if (root != NULL) {
	tdestroy_recurse (root, freefct);
    }
}
libc_hidden_def(tdestroy)
#endif
#endif

/* tsearch.c ends here */
