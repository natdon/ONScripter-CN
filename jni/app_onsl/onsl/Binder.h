/*
* Copyright (c) 2010, Helios (helios.vmg@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the author may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*     * Products derived from this software may not be called "ONSlaught" nor
*       may "ONSlaught" appear in their names without specific prior written
*       permission from the author.
*
* THIS SOFTWARE IS PROVIDED BY HELIOS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL HELIOS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BINDER_H
#define BINDER_H
template <typename FT,typename PT0=char,typename PT1=char,typename PT2=char,typename PT3=char,typename PT4=char,typename PT5=char,typename PT6=char,typename PT7=char,typename PT8=char,typename PT9=char>
struct binder{
	FT f;
	PT0 pt0;
	PT1 pt1;
	PT2 pt2;
	PT3 pt3;
	PT4 pt4;
	PT5 pt5;
	PT6 pt6;
	PT7 pt7;
	PT8 pt8;
	PT9 pt9;
	int p;
	bool free_after_first_use;
	typedef void(*Call_1)(PT0);
	typedef void(*Call_2)(PT0,PT1);
	typedef void(*Call_3)(PT0,PT1,PT2);
	typedef void(*Call_4)(PT0,PT1,PT2,PT3);
	typedef void(*Call_5)(PT0,PT1,PT2,PT3,PT4);
	typedef void(*Call_6)(PT0,PT1,PT2,PT3,PT4,PT5);
	typedef void(*Call_7)(PT0,PT1,PT2,PT3,PT4,PT5,PT6);
	typedef void(*Call_8)(PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7);
	typedef void(*Call_9)(PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8);
	typedef void(*Call_10)(PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8,PT9);
	binder():f(0),p(0),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0):f(f),pt0(pt0),p(1),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1):f(f),pt0(pt0),pt1(pt1),p(2),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2):f(f),pt0(pt0),pt1(pt1),pt2(pt2),p(3),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),p(4),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),p(5),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),p(6),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),p(7),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),pt7(pt7),p(8),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),pt7(pt7),pt8(pt8),p(9),free_after_first_use(1){}
	binder(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8,const PT9 &pt9):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),pt7(pt7),pt8(pt8),pt9(pt9),p(10),free_after_first_use(1){}
	void call(){
		if (p<6){
			if (p<3){
				if (p<2){
					((Call_1)f)(pt0);
				}else{
					((Call_2)f)(pt0,pt1);
				}
			}else if (p>3){
				if (p<5){
					((Call_4)f)(pt0,pt1,pt2,pt3);
				}else{
					((Call_5)f)(pt0,pt1,pt2,pt3,pt4);
				}
			}else{
				((Call_3)f)(pt0,pt1,pt2);
			}
		}else if (p>6){
			if (p<9){
				if (p<8){
					((Call_7)f)(pt0,pt1,pt2,pt3,pt4,pt5,pt6);
				}else{
					((Call_8)f)(pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7);
				}
			}else if (p>9){
				((Call_10)f)(pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8,pt9);
			}else{
				((Call_9)f)(pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8);
			}
		}else{
			((Call_6)f)(pt0,pt1,pt2,pt3,pt4,pt5);
		}
	}
};

template <typename FT,typename PT0>
inline binder<FT,PT0> *bind(FT f,const PT0 &pt0){
	return new binder<FT,PT0>(f,pt0);
}
template <typename FT,typename PT0,typename PT1>
inline binder<FT,PT0,PT1> *bind(FT f,const PT0 &pt0,const PT1 &pt1){
	return new binder<FT,PT0,PT1>(f,pt0,pt1);
}
template <typename FT,typename PT0,typename PT1,typename PT2>
inline binder<FT,PT0,PT1,PT2> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2){
	return new binder<FT,PT0,PT1,PT2>(f,pt0,pt1,pt2);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3>
inline binder<FT,PT0,PT1,PT2,PT3> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3){
	return new binder<FT,PT0,PT1,PT2,PT3>(f,pt0,pt1,pt2,pt3);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4>
inline binder<FT,PT0,PT1,PT2,PT3,PT4> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4){
	return new binder<FT,PT0,PT1,PT2,PT3,PT4>(f,pt0,pt1,pt2,pt3,pt4);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5>
inline binder<FT,PT0,PT1,PT2,PT3,PT4,PT5> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5){
	return new binder<FT,PT0,PT1,PT2,PT3,PT4,PT5>(f,pt0,pt1,pt2,pt3,pt4,pt5);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6>
inline binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6){
	return new binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6,typename PT7>
inline binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7){
	return new binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6,typename PT7,typename PT8>
inline binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8){
	return new binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6,typename PT7,typename PT8,typename PT9>
inline binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8,PT9> *bind(FT f,const PT0 &pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8,const PT9 &pt9){
	return new binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8,PT9>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8,pt9);
}
#define BINDER_TYPEDEF_1(name,arg1) typedef binder<void(*)(arg1),arg1> name
#define BINDER_TYPEDEF_2(name,arg1,arg2) typedef binder<void(*)(arg1,arg2),arg1,arg2> name
#define BINDER_TYPEDEF_3(name,arg1,arg2,arg3) typedef binder<void(*)(arg1,arg2,arg3),arg1,arg2,arg3> name
#define BINDER_TYPEDEF_4(name,arg1,arg2,arg3,arg4) typedef binder<void(*)(arg1,arg2,arg3,arg4),arg1,arg2,arg3,arg4> name
#define BINDER_TYPEDEF_5(name,arg1,arg2,arg3,arg4,arg5) typedef binder<void(*)(arg1,arg2,arg3,arg4,arg5),arg1,arg2,arg3,arg4,arg5> name
#define BINDER_TYPEDEF_6(name,arg1,arg2,arg3,arg4,arg5,arg6) typedef binder<void(*)(arg1,arg2,arg3,arg4,arg5,arg6),arg1,arg2,arg3,arg4,arg5,arg6> name
#define BINDER_TYPEDEF_7(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7) typedef binder<void(*)(arg1,arg2,arg3,arg4,arg5,arg6,arg7),arg1,arg2,arg3,arg4,arg5,arg6,arg7> name
#define BINDER_TYPEDEF_8(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) typedef binder<void(*)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8),arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8> name
#define BINDER_TYPEDEF_9(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) typedef binder<void(*)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9),arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9> name
#define BINDER_TYPEDEF_10(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10) typedef binder<void(*)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10),arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10> name

template <typename FT,typename PT0=char,typename PT1=char,typename PT2=char,typename PT3=char,typename PT4=char,typename PT5=char,typename PT6=char,typename PT7=char,typename PT8=char,typename PT9=char>
struct member_binder{
	FT f;
	PT0 *pt0;
	PT1 pt1;
	PT2 pt2;
	PT3 pt3;
	PT4 pt4;
	PT5 pt5;
	PT6 pt6;
	PT7 pt7;
	PT8 pt8;
	PT9 pt9;
	int p;
bool free_after_first_use;
	typedef void(PT0::*Call_1)();
	typedef void(PT0::*Call_2)(PT1);
	typedef void(PT0::*Call_3)(PT1,PT2);
	typedef void(PT0::*Call_4)(PT1,PT2,PT3);
	typedef void(PT0::*Call_5)(PT1,PT2,PT3,PT4);
	typedef void(PT0::*Call_6)(PT1,PT2,PT3,PT4,PT5);
	typedef void(PT0::*Call_7)(PT1,PT2,PT3,PT4,PT5,PT6);
	typedef void(PT0::*Call_8)(PT1,PT2,PT3,PT4,PT5,PT6,PT7);
	typedef void(PT0::*Call_9)(PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8);
	typedef void(PT0::*Call_10)(PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8,PT9);
	member_binder(FT f,PT0 *pt0):f(f),pt0(pt0),p(1),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1):f(f),pt0(pt0),pt1(pt1),p(2),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2):f(f),pt0(pt0),pt1(pt1),pt2(pt2),p(3),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),p(4),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),p(5),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),p(6),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),p(7),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),pt7(pt7),p(8),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),pt7(pt7),pt8(pt8),p(9),free_after_first_use(1){}
	member_binder(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8,const PT9 &pt9):f(f),pt0(pt0),pt1(pt1),pt2(pt2),pt3(pt3),pt4(pt4),pt5(pt5),pt6(pt6),pt7(pt7),pt8(pt8),pt9(pt9),p(10),free_after_first_use(1){}
	void call(){
		if (p<6){
			if (p<3){
				if (p<2){
					(pt0->*(Call_1)f)();
				}else{
					(pt0->*(Call_2)f)(pt1);
				}
			}else if (p>3){
				if (p<5){
					(pt0->*(Call_4)f)(pt1,pt2,pt3);
				}else{
					(pt0->*(Call_5)f)(pt1,pt2,pt3,pt4);
				}
			}else{
				(pt0->*(Call_3)f)(pt1,pt2);
			}
		}else if (p>6){
			if (p<9){
				if (p<8){
					(pt0->*(Call_7)f)(pt1,pt2,pt3,pt4,pt5,pt6);
				}else{
					(pt0->*(Call_8)f)(pt1,pt2,pt3,pt4,pt5,pt6,pt7);
				}
			}else if (p>9){
				(pt0->*(Call_10)f)(pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8,pt9);
			}else{
				(pt0->*(Call_9)f)(pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8);
			}
		}else{
			(pt0->*(Call_6)f)(pt1,pt2,pt3,pt4,pt5);
		}
	}
};

template <typename FT,typename PT0>
inline member_binder<FT,PT0> *member_bind(FT f,PT0 *pt0){
	return new member_binder<FT,PT0>(f,pt0);
}
template <typename FT,typename PT0,typename PT1>
inline member_binder<FT,PT0,PT1> *member_bind(FT f,PT0 *pt0,const PT1 &pt1){
	return new member_binder<FT,PT0,PT1>(f,pt0,pt1);
}
template <typename FT,typename PT0,typename PT1,typename PT2>
inline member_binder<FT,PT0,PT1,PT2> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2){
	return new member_binder<FT,PT0,PT1,PT2>(f,pt0,pt1,pt2);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3>
inline member_binder<FT,PT0,PT1,PT2,PT3> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3){
	return new member_binder<FT,PT0,PT1,PT2,PT3>(f,pt0,pt1,pt2,pt3);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4>
inline member_binder<FT,PT0,PT1,PT2,PT3,PT4> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4){
	return new member_binder<FT,PT0,PT1,PT2,PT3,PT4>(f,pt0,pt1,pt2,pt3,pt4);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5>
inline member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5){
	return new member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5>(f,pt0,pt1,pt2,pt3,pt4,pt5);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6>
inline member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6){
	return new member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6,typename PT7>
inline member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7){
	return new member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6,typename PT7,typename PT8>
inline member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8){
	return new member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8);
}
template <typename FT,typename PT0,typename PT1,typename PT2,typename PT3,typename PT4,typename PT5,typename PT6,typename PT7,typename PT8,typename PT9>
inline member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8,PT9> *member_bind(FT f,PT0 *pt0,const PT1 &pt1,const PT2 &pt2,const PT3 &pt3,const PT4 &pt4,const PT5 &pt5,const PT6 &pt6,const PT7 &pt7,const PT8 &pt8,const PT9 &pt9){
	return new member_binder<FT,PT0,PT1,PT2,PT3,PT4,PT5,PT6,PT7,PT8,PT9>(f,pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8,pt9);
}
#define MEMBER_BINDER_TYPEDEF_1(name,arg1) typedef member_binder<void(arg1::*)(),arg1> name
#define MEMBER_BINDER_TYPEDEF_2(name,arg1,arg2) typedef member_binder<void(arg1::*)(arg2),arg1,arg2> name
#define MEMBER_BINDER_TYPEDEF_3(name,arg1,arg2,arg3) typedef member_binder<void(arg1::*)(arg2,arg3),arg1,arg2,arg3> name
#define MEMBER_BINDER_TYPEDEF_4(name,arg1,arg2,arg3,arg4) typedef member_binder<void(arg1::*)(arg2,arg3,arg4),arg1,arg2,arg3,arg4> name
#define MEMBER_BINDER_TYPEDEF_5(name,arg1,arg2,arg3,arg4,arg5) typedef member_binder<void(arg1::*)(arg2,arg3,arg4,arg5),arg1,arg2,arg3,arg4,arg5> name
#define MEMBER_BINDER_TYPEDEF_6(name,arg1,arg2,arg3,arg4,arg5,arg6) typedef member_binder<void(arg1::*)(arg2,arg3,arg4,arg5,arg6),arg1,arg2,arg3,arg4,arg5,arg6> name
#define MEMBER_BINDER_TYPEDEF_7(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7) typedef member_binder<void(arg1::*)(arg2,arg3,arg4,arg5,arg6,arg7),arg1,arg2,arg3,arg4,arg5,arg6,arg7> name
#define MEMBER_BINDER_TYPEDEF_8(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) typedef member_binder<void(arg1::*)(arg2,arg3,arg4,arg5,arg6,arg7,arg8),arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8> name
#define MEMBER_BINDER_TYPEDEF_9(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) typedef member_binder<void(arg1::*)(arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9),arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9> name
#define MEMBER_BINDER_TYPEDEF_10(name,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10) typedef member_binder<void(arg1::*)(arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10),arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10> name

template <typename T>
void call_bound(void *p){
	T *binder=(T *)p;
	binder->call();
	if (binder->free_after_first_use)
		delete binder;
}
#endif
