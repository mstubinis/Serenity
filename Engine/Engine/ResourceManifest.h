#pragma once
#ifndef RESOURCE_MANIFEST_H
#define RESOURCE_MANIFEST_H

struct Handle;

class ResourceManifest{
    public:
		static Handle skyFromSpace;
		static Handle skyFromAtmosphere;
		static Handle groundFromSpace;

		static void init();
};

#endif