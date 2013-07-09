// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_IMAGE_TRANSPORT_FACTORY_H_
#define CONTENT_BROWSER_RENDERER_HOST_IMAGE_TRANSPORT_FACTORY_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "ui/gfx/native_widget_types.h"

namespace gfx {
class Size;
}

namespace ui {
class ContextFactory;
class Texture;
}

namespace WebKit {
class WebGraphicsContext3D;
}

namespace content {
class GLHelper;

// This class provides a way to get notified when surface handles get lost.
class ImageTransportFactoryObserver {
 public:
  virtual ~ImageTransportFactoryObserver() {}

  // Notifies that the surface handles generated by ImageTransportFactory were
  // lost.
  // When this is called, the old resources (e.g. shared context, GL helper)
  // still exist, but are about to be destroyed. Getting a reference to those
  // resources from the ImageTransportFactory (e.g. through GetGLHelper) will
  // return newly recreated, valid resources.
  virtual void OnLostResources() = 0;
};

// This class provides the interface for creating the support for the
// cross-process image transport, both for creating the shared surface handle
// (destination surface for the GPU process) and the transport client (logic for
// using that surface as a texture). The factory is a process-wide singleton.
// As this is intimately linked to the type of 3D context we use (in-process or
// command-buffer), implementations of this also implement ui::ContextFactory.
class ImageTransportFactory {
 public:
  virtual ~ImageTransportFactory() {}

  // Initialize the global transport factory.
  static void Initialize();

  // Terminates the global transport factory.
  static void Terminate();

  // Gets the factory instance.
  static ImageTransportFactory* GetInstance();

  // Gets the image transport factory as a context factory for the compositor.
  virtual ui::ContextFactory* AsContextFactory() = 0;

  // Creates a shared surface handle.
  // Note: the handle may get lost at any time, a state that an
  // ImageTransportFactoryObserver gets notified of.
  virtual gfx::GLSurfaceHandle CreateSharedSurfaceHandle() = 0;

  // Destroys a shared surface handle.
  virtual void DestroySharedSurfaceHandle(gfx::GLSurfaceHandle surface) = 0;

  // Creates a transport texture of a given size and scale factor, and using the
  // opaque handle sent by the GPU process.
  virtual scoped_refptr<ui::Texture> CreateTransportClient(
      const gfx::Size& size,
      float device_scale_factor,
      const std::string& mailbox_name) = 0;

  // Variant of CreateTransportClient() that deletes the texture on the GPU when
  // the returned value is deleted.
  virtual scoped_refptr<ui::Texture> CreateOwnedTexture(
      const gfx::Size& size,
      float device_scale_factor,
      unsigned int texture_id) = 0;

  // Gets a GLHelper instance, associated with the shared context. This
  // GLHelper will get destroyed whenever the shared context is lost
  // (ImageTransportFactoryObserver::OnLostResources is called).
  virtual GLHelper* GetGLHelper() = 0;

  // Inserts a SyncPoint into the shared context.
  virtual uint32 InsertSyncPoint() = 0;

  virtual void AddObserver(ImageTransportFactoryObserver* observer) = 0;
  virtual void RemoveObserver(ImageTransportFactoryObserver* observer) = 0;
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_IMAGE_TRANSPORT_FACTORY_H_