/*
 * Copyright (C) 1999-2011 Insight Software Consortium
 * Copyright (C) 2005-2024 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _itkPyCommand_h
#define _itkPyCommand_h

#include "itkCommand.h"

// The python header defines _POSIX_C_SOURCE without a preceding #undef
#undef _POSIX_C_SOURCE
// The python header defines _XOPEN_SOURCE without a preceding #undef
#undef _XOPEN_SOURCE

#include <Python.h>

namespace itk
{

/** \class PyCommand
 *  \brief Command subclass that calls a Python callable object, e.g.
 *  a Python function.
 *
 * With this class, arbitrary Python callable objects (e.g. functions)
 * can be associated with an instance to be used in AddObserver calls.
 * This is analogous to itk::TclCommand, but then a tad more flexible.; )
 *
 * This class was contributed by Charl P. Botha <cpbotha |AT| ieee.org>
 *
 * \ingroup OTBSWIG
 */
class PyCommand : public Command
{
public:
  ///! Standard "Self" typedef.
  typedef PyCommand Self;

  ///! Smart pointer typedef support.
  typedef SmartPointer<Self> Pointer;

  ///! Run-time type information (and related methods).
  itkTypeMacro(PyCommand, Command);

  ///! Method for creation through the object factory.
  itkNewMacro(Self);

  /**
   * Assign a Python callable object to be used.  You don't have to keep
   * a binding to the callable, PyCommand will also take out a reference
   * to make sure the Callable sticks around.
   */
  void SetCommandCallable(PyObject* obj);

  PyObject* GetCommandCallable();

  void Execute(Object*, const EventObject&) override;
  void Execute(const Object*, const EventObject&) override;

protected:
  PyCommand();
  ~PyCommand();
  void PyExecute();
  PyCommand(const Self&);      // Not implemented.
  void operator=(const Self&); // Not implemented.

private:
  PyObject* obj;
};


} // namespace itk

#endif // _itkPyCommand_h
