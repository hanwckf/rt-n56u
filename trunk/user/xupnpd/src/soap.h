/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
 */

#ifndef __SOAP_H
#define __SOAP_H

namespace soap
{
    class string
    {
    protected:
        char* ptr;
        int size;
    public:
        string(void):ptr(0),size(0) {}
        ~string(void) { clear(); }

        void clear(void);

        int length(void) const { return size; }
        const char* c_str(void) const { return ptr?ptr:""; }

        void swap(string& s);

        void trim_right(void);

        friend class string_builder;
        friend class ctx;
    };

    struct chunk
    {
        enum { max_size=64 };

        char* ptr;
        int size;

        chunk* next;
    };

    class string_builder
    {
    protected:
        chunk *beg,*end;

        int add_chunk(void);

    public:
        string_builder(void):beg(0),end(0) {}
        ~string_builder(void) { clear(); }

        void clear(void);

        void add(int ch)
        {
            if(!end || end->size>=chunk::max_size)
                if(add_chunk())
                    return;
            ((unsigned char*)end->ptr)[end->size++]=ch;
        }

        void add(const char* s,int len);

        void swap(string& s);
    };


    struct node
    {
        node* parent;
        node* next;
        node* beg;
        node* end;

        char* name;
        char* data;
        int   len;

        char* attr;
        int   attr_len;

        node(void):parent(0),next(0),beg(0),end(0),name(0),data(0),len(0),attr(0),attr_len(0) {}
        ~node(void) { clear(); }

        void init(void) { parent=next=beg=end=0; name=data=attr=0; len=attr_len=0; }

        node* add_node(void);

        void clear(void);

        node* find_child(const char* s,int l);
        node* find(const char* s);

        const char* operator[](const char* s)
        {
            return find_data(s);
        }

        const char* find_data(const char* s)
        {
            node* pp=find(s);

            if(pp && pp->data)
                return pp->data;
            return "";
        }
    };


    class ctx
    {
    protected:
        node* cur;

        short st;
        short st_close_tag;
        short st_quot;
        short st_text;
        short err;

        string_builder data;

        enum { max_tok_size=64 };

        char tok[max_tok_size];
        int tok_size;

        void tok_add(unsigned char ch)
        {
            if(tok_size<max_tok_size-1)
                ((unsigned char*)tok)[tok_size++]=ch;
        }
        void tok_reset(void) { tok_size=0; }

        void ch_push(unsigned char ch) { data.add(ch); }
        void tok_push(void);
        void tag_open(const char* s,int len);
        void tag_close(const char* s,int len);
        void data_push(void);
        void attr_push(void);

    public:
        int line;
        int attributes;
    public:
        ctx(node* root):cur(root),st(0),err(0),line(0),tok_size(0),st_close_tag(0),st_quot(0),st_text(0),attributes(0) {}

        void begin(void);

        int parse(const char* buf,int len);

        int end(void);
    };


    int parse(const char* s,int l,node* root);
}

#endif /* __SOAP_H */
