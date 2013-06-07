# This file is used to generate Binder.h. To use it, run
# python binder.py > Binder.h

import math

def binder_constructor(n):
	output="\tbinder(FT f"
	for i in range(n):
		output+=",const PT%d &pt%d"%(i,i)
	output+="):f(f),"
	for i in range(n):
		output+="pt%d(pt%d),"%(i,i)
	output+="p(%d),free_after_first_use(1){}\n"%n
	return output

def do_case(i,indent):
	output=""
	output+=indent+"((Call_%d)f)("%i
	j=0
	for j in range(i):
		output+="pt%d"%j
		if j<i-1:
			output+=","
	output+=");"
	return output

def efficient_if(begin,end,indent):
	output=""
	indent+="\t"
	
	if begin>=end:
		output+=do_case(begin,indent)
	else:
		pivot=int(math.ceil((end+begin)/2))
		flag=0
		output+=indent
		if pivot>begin:
			output+="if (p<%d){\n"%pivot
			output+=efficient_if(begin,pivot-1,indent)
			output+=indent+"}else"
			flag=1;
		if pivot<end:
			if flag:
				output+=" "
			output+="if (p>%d){\n"%pivot
			output+=efficient_if(pivot+1,end,indent)
			output+=indent+"}else"
			flag=1
		output+="{\n"
		output+=do_case(pivot,indent+"\t")
		output+="\n"+indent+"}"
	output+="\n"
	return output

def binder_class(n):
	output=""
	output+="template <typename FT"
	for i in range(n):
		output+=",typename PT%d=char"%i
	output+=">\nstruct binder{\n\tFT f;\n"
	for i in range(n):
		output+="\tPT%d pt%d;\n"%(i,i)
	output+="\tint p;\n\tbool free_after_first_use;\n"
	for i in range(1,n+1):
		string=""
		for j in range(0,i-1):
			string+="PT%d,"%j
		output+="\ttypedef void(*Call_%d)(%sPT%d);\n"%(i,string,i-1)
	output+="\tbinder():f(0),p(0),free_after_first_use(1){}\n"
	for i in range(1,n+1):
		output+=binder_constructor(i)
	output+="\tvoid call(){\n"
	output+=efficient_if(1,n,"\t");
	output+="\t}\n};\n\n"
	
	for i in range(n):
		template_parameters="template <typename FT"
		return_type="binder<FT"
		parameters="FT f"
		parameters_passed="f"
		for j in range(i+1):
			template_parameters+=",typename PT%d"%j
			return_type+=",PT%d"%j
			parameters+=",const PT%d &pt%d"%(j,j)
			parameters_passed+=",pt%d"%j
		template_parameters+=">"
		return_type+=">"
		#output+=template_parameters+"\n"+return_type+" *bind("+parameters+"){\n\treturn new "+return_type+"("+parameters_passed+");\n}\n"
		output+="%s\ninline %s *bind(%s){\n\treturn new %s(%s);\n}\n"%(template_parameters,return_type,parameters,return_type,parameters_passed)
	
	for i in range(1,n+1):
		string=""
		j=0
		for j in range(1,i):
			string+="arg%d,"%j
		string+="arg%d"%(j+1)
		output+="#define BINDER_TYPEDEF_%d(name,%s) typedef binder<void(*)(%s),%s> name\n"%(i,string,string,string)
	
	return output

def member_binder_constructor(n):
	output="\tmember_binder(FT f,PT0 *pt0"
	for i in range(1,n):
		output+=",const PT%d &pt%d"%(i,i)
	output+="):f(f),"
	for i in range(n):
		output+="pt%d(pt%d),"%(i,i)
	output+="p(%d),free_after_first_use(1){}\n"%n
	return output

def do_member_case(i,indent):
	output=""
	output+=indent+"(pt0->*(Call_%d)f)("%i
	for j in range(1,i):
		output+="pt%d"%j
		if j<i-1:
			output+=","
	output+=");"
	return output

def member_efficient_if(begin,end,indent):
	output=""
	indent+="\t"
	
	if begin>=end:
		output+=do_member_case(begin,indent)
	else:
		pivot=int(math.ceil((end+begin)/2))
		flag=0
		output+=indent
		if pivot>begin:
			output+="if (p<%d){\n"%pivot
			output+=member_efficient_if(begin,pivot-1,indent)
			output+=indent+"}else"
			flag=1;
		if pivot<end:
			if flag:
				output+=" "
			output+="if (p>%d){\n"%pivot
			output+=member_efficient_if(pivot+1,end,indent)
			output+=indent+"}else"
			flag=1
		output+="{\n"
		output+=do_member_case(pivot,indent+"\t")
		output+="\n"+indent+"}"
	output+="\n"
	return output

def member_binder_class(n):
	output=""
	output+="template <typename FT"
	for i in range(n):
		output+=",typename PT%d=char"%i
	output+=">\nstruct member_binder{\n\tFT f;\n\tPT0 *pt0;\n"
	for i in range(1,n):
		output+="\tPT%d pt%d;\n"%(i,i)
	output+="\tint p;\nbool free_after_first_use;\n"
	for i in range(1,n+1):
		string=""
		for j in range(1,i-1):
			string+="PT%d,"%j
		if i>1:
			string+="PT%d"%(i-1);
		output+="\ttypedef void(PT0::*Call_%d)(%s);\n"%(i,string)
	for i in range(1,n+1):
		output+=member_binder_constructor(i)
	output+="\tvoid call(){\n"
	output+=member_efficient_if(1,n,"\t");
	output+="\t}\n};\n\n"
	
	for i in range(n):
		template_parameters="template <typename FT"
		return_type="member_binder<FT"
		parameters="FT f,PT0 *pt0"
		parameters_passed="f"
		for j in range(1,i+1):
			parameters+=",const PT%d &pt%d"%(j,j)
		for j in range(i+1):
			template_parameters+=",typename PT%d"%j
			return_type+=",PT%d"%j
			parameters_passed+=",pt%d"%j
		template_parameters+=">"
		return_type+=">"
		output+="%s\ninline %s *member_bind(%s){\n\treturn new %s(%s);\n}\n"%(template_parameters,return_type,parameters,return_type,parameters_passed)
	
	for i in range(1,n+1):
		fp=""
		j=0
		for j in range(2,i+1):
			fp+="arg%d"%j
			if j<i:
				fp+=","
		string="arg1"
		if len(fp)>0:
			string+=","
		string+=fp
		output+="#define MEMBER_BINDER_TYPEDEF_%d(name,%s) typedef member_binder<void(arg1::*)(%s),%s> name\n"%(i,string,fp,string)
	
	return output

def binders(n):
	return binder_class(n)+"\n"+member_binder_class(n)

def main():
	print("/*")
	print("* Copyright (c) 2010, Helios (helios.vmg@gmail.com)")
	print("* All rights reserved.")
	print("*")
	print("* Redistribution and use in source and binary forms, with or without")
	print("* modification, are permitted provided that the following conditions are met:")
	print("*     * Redistributions of source code must retain the above copyright notice," )
	print("*       this list of conditions and the following disclaimer.")
	print("*     * Redistributions in binary form must reproduce the above copyright")
	print("*       notice, this list of conditions and the following disclaimer in the")
	print("*       documentation and/or other materials provided with the distribution.")
	print("*     * The name of the author may not be used to endorse or promote products")
	print("*       derived from this software without specific prior written permission.")
	print("*     * Products derived from this software may not be called \"ONSlaught\" nor")
	print("*       may \"ONSlaught\" appear in their names without specific prior written")
	print("*       permission from the author.")
	print("*")
	print("* THIS SOFTWARE IS PROVIDED BY HELIOS \"AS IS\" AND ANY EXPRESS OR IMPLIED")
	print("* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF")
	print("* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO")
	print("* EVENT SHALL HELIOS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,")
	print("* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,")
	print("* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;")
	print("* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,")
	print("* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR")
	print("* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED")
	print("* OF THE POSSIBILITY OF SUCH DAMAGE.")
	print("*/\n")
	print("#ifndef BINDER_H")
	print("#define BINDER_H")
	print(binders(10))
	print("template <typename T>")
	print("void call_bound(void *p){")
	print("\tT *binder=(T *)p;")
	print("\tbinder->call();")
	print("\tif (binder->free_after_first_use)")
	print("\t\tdelete binder;")
	print("}")
	print("#endif")

main()
