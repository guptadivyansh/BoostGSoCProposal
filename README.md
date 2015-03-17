#Enhanced Vector and Deque Containers Proposal

##GSoC 2015 Project for Boost Library


##Personal Details

  * **Name:** Divyansh Gupta
  * **University:** Indian Institute of Technology, Kharagpur
  * **Course/Major:** Computer Science and Engineering
  * **Degree Program:** Integrated B.Tech. + M.Tech
  * **Email:** <divyanshgupta95@gmail.com>
  * **Availability:** I intend to spend 40 hours per week in the summer. However my college reopens on 13th July and after that I'll be able to manage only 15-20 hours a week due to coursework. On the flip side, my college closes on 29th April, which is roughly one month before the GSoC coding period starts. Hence, I'd prefer to start working as early as possible, subject to mentor availability.

##Background Information


**Please summarize your educational background (degrees earned, courses taken, etc.).**

I'm currently a second year student (out of 5) of a B.Tech + M.Tech program in Computer Science and Engineering.
A few completed courses relevant to this project are: 
  
* **Software Engineering (+Lab)** - A two part course where the first part covered all the basic features of C++98 and the second part covered software development procedures like UML designing, testing, documenting, etc. The lab involved creating fraction, polynomial and (a very trivial) deque libraries in C++ apart from a few GUI-based softwares in Java.
* **Algorithms (+Lab)** - Covered complexity analysis and design of basic algorithms and data structures like binary search trees, RB trees, heaps, hash tables etc.

**Please summarize your programming background (OSS projects, internships, jobs, etc.).**

I'm mainly interested in the field of robotics and AI. I'm a part of the Swarm Robotics research group in my college where I write code (in C++), and using ROS (Robot Operating System), control the various behaviours of robots in Gazebo simulator or in real life.

**Please tell us a little about your programming interests. Please tell us why you are interested in contributing to the Boost C++ Libraries.**

I've been using several OSS projects in my daily life for several years, and my motivation to contribute to them some day was one of the reasons I joined a Computer Science program in college. I believe that I have acquired sufficient proficiency in the last two years that I can start giving back to the OSS community, and since C++ is my forte, Boost C++ Library would be the best place for me to step into the world of OSS.

**What is your interest in the project you are proposing?**

My main motivation in doing this project, apart from contributing to the open source community, is to learn how to write low-level exception-safe C++ code of extremely high quality, that Boost is known for. Just doing the programming competency test has exposed me to the intricate world of C++11/STL and has made me a better programmer for sure. It has made me realize how valuable a GSoC in Boost would be for me.

I'm more interested in the `devector` container since `std::vector` is the most popular STL container and creating a `boost::container::devector` with even more flexibility over it, would greatly benefit the C++ community in general. In fact, when this project was initially discussed on the mailing list (when Boost was applying as an organization), it didn't include the double-ended vector idea. I suggested it to the mentor and he included it on the final ideas page.

**Have you done any previous work in this area before or on similar projects?**

I have made simple, safe containers as wrappers around arrays, but never a completely STL-compliant container. However, the programming competency test was a good warmup for the project and I believe that it has made me well equipped with the basics required to tackle the problem at hand. 

**What are your plans beyond this Summer of Code time frame for your proposed work?.**

Apart from supporting and incorporating suggestions to the containers proposed below, I have plans for another double ended container that uses a dynamically growing circular buffer to store its elements. This would be beneficial if a user requires a FIFO like contiguous container. (The `devector` will be inefficient since it'll have to reallocate or at least copy elements even if capacity is reached on just one side).

Also, depending on how the community reacts to the proposed devector, a `stable_devector` can be developed similar to the `stable_vector` in Boost, which would sacrifice contiguity for reference stability.

**Please rate, from 0 to 5 (0 being no experience, 5 being expert), your knowledge of the following languages, technologies, or tools:**

(The following are rated according to an undergrad's level)

* **C++ 98/03:** 4, well acquainted with all language features required for this project, since I use it for my robotics projecs and it was recently covered in the course mentioned above.
* **C++ 11/14:** 2.5, self-taught, mostly for the programming competency test. Comfortable with move semantics, auto, lambdas, but still a lot to learn.
* **C++ STL:** 3.5, have been using the containers and algorithms for some time now. Studied allocators and iterators for the competency test. I also learned how `std::vector` and `std::deque` implementations store their data internally and their [complexity analysis][vector-analysis].
* **Boost C++ Libraries:** 1, I'm new to Boost and have only used the Boost Test library a few times.
* **Git:** 3.5, have been using git for my personal and group projects.


**What software development environments are you most familiar with (Visual Studio, Eclipse, KDevelop, etc.)?**

I am most comfortable with using Sublime Text Editor and GCC compiler on the command line but recently I've started using Eclipse CDT for development. I prefer to use Linux (Ubuntu) for development.

**What software documentation tool are you most familiar with (Doxygen, DocBook, Quickbook, etc.)?**

I'm familiar with basic Doxygen usage, but I'll have to learn how Boost prefers to document its code.

##Project Proposal

###Devector

I propose to write a new `devector` container that is fully STL compliant and extends `std::vector` in a few crucial aspects:

* Provides a `push_front` function which guarantees amortized O(1) insertions at the front. This will be done by inserting elements from the middle in the allocated space and growing similar to how `std::vector` grows on `push_back`.
* Provides other functions like `emplace_front` and `pop_front` that mimic their corresponding "back" functions at the front. 
* Provides `reserve_front` and `reserve_back` functions to allow the user to reserve space at the front or back before doing multiple `push_front`'s or `push_back`'s to possibly limit the number of reallocations required (and hence prevent frequent invalidation of references). 
* Provides a `set_growth_factor(double m)` function, where m > 1.0, to let experienced users determine the compromise between wasted space and number of reallocations. (A high growth factor leads to more wasted space and a growth factor close to 1.0 leads to very frequent reallocations). Currently, the growth factor of vectors is implementation dependent and cannot be set by the user. For example, [`boost::container::vector`][boost-vector] has a growth factor of 1.6.

Since we're rethinking such a basic STL container like vector, community feedback will be essential in considering additional features that users have felt to be lacking in `std::vector`. The goal is to create a double-ended vector that gives more flexibilty to users while preserving the interface they are familiar with. The container will mimic the behaviour of `std::vector` for the functions they have in common, so that the `devector` can be used as a drop-in replacement for `std::vector`, while providing the extra functionality mentioned above. So, for example, `reserve()` would be an alias for `reserve_back()`, to replicate the behavior of `std::vector::reserve()`.

####Drawbacks

The main drawback of `devector`, as with normal `vector`'s, is that all references are potentially invalidated on operations like `push_back`, `push_front`, `insert`, `emplace`, etc. Also, if the growth factor is too large, then a lot of memory will be wasted, especially since it allocates extra free space on both sides compared to a normal `vector`.

###Deque

In certain applications, reference stability is a much more important criteria than memory contiguity. That is where `std::deque` comes into the picture. I propose to add the following features to `boost::container::deque` to make them even more flexible.

* Ability to set the size of the segments in the constructors
* `reserve_front` and `reserve_back` functions that allow the user to reserve space before pushing elements to the front or back.
* Fast iteration via a member function called `for_each` or `apply_for_each`

After consulting the mentor, it was decided that it would be more useful to have the above function, rather than a new category of segmented iterator. 

Since `boost::container::deque` is meant to mirror the standard container, rather than making changes to it, I intend on reusing as much of that code as possible and creating a new `flexible_deque` container which incorporates the above features while preserving the current functionality. This will require a very thorough study of the `boost::container::deque` code.


##Proposed Milestones and Schedule

As mentioned above, I'd prefer to start early and finish up early to avoid clashes with coursework. 

  * **May 5th**: Finalize the interface and features with the mentor.
  * **May 15th**: Study [`boost::container::vector`][boost-vector] and implementations of `std::vector` and `std::deque`. Another 3rd party devector implementation I have looked at can be found [here][3rd-vector]. 
  * **May 22nd**: Finish writing the basic structure of the `devector` container, including the constructors, destructor, assignment operators, iterators and accessors.
  * **June 5th**: Write the rest of the STL interface and the new modifiers like push_front, reserve_front, etc.
  * **June 19th**: Test all the functions of the `devector` for memory leaks and exception guarantees. Also, benchmark the performance of the new container against `std::vector`and `std::deque`. (Inspired from [here][benchmark])
  * **June 26th**: Request for community feedback and incorporate suggestions. Buffer time for any unforeseen delays.

**_June 27th - July 4th Mid-term evaluation deliverables_:** completed `devector` along with tests and benchmarking report.
  
  * **July 3rd**: Study [`boost::container::deque`][boost-deque] and implementations of `std::deque`. Another 3rd party deque implementation I have looked at can be found [here][3rd-deque]. Determine how much code can be reused from `boost::container::deque`.
  * **July 17th**: Incorporate the new features without breaking deque::container::deque.
  * **July 31st**: Run all the earlier [tests][deque-tests] of `boost::container::deque` to ensure nothing broke and write tests for the new features. Also, benchmark the performance of the new container against `std::deque`.
  * **August 7th**: Request for community feedback and incorporate suggestions. Buffer time for any unforeseen delays.
  * **August 14th**: Write documentation. Begin working on the dynamic circular buffer or `stable_devector` depending on community enthusiasm.

##Programming Competency

My solution to the programming competency test is available [here][competency-test]

[vector-analysis]: http://stackoverflow.com/questions/6550509/amortized-analysis-of-stdvector-insertion
[3rd-vector]: https://github.com/orlp/devector
[3rd-deque]: http://lists.boost.org/Archives/boost/2009/11/159373.php
[boost-vector]: http://www.boost.org/doc/libs/1_57_0/boost/container/vector.hpp
[boost-deque]: http://www.boost.org/doc/libs/1_57_0/boost/container/deque.hpp
[benchmark]: http://www.codeproject.com/Articles/5425/An-In-Depth-Study-of-the-STL-Deque-Container
[deque-tests]: https://github.com/boostorg/container/blob/master/test/deque_test.cpp
[competency-test]: https://github.com/guptadivyansh/BoostGSoCProposal

