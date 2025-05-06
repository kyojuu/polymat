#ifndef INDEXVECTOR_H
#define INDEXVECTOR_H

#include <vector>
#include <cstdint>

namespace civ 
{
	using ID = uint64_t;

	template<typename T>
	struct Ref;

	template<typename T>
	struct PRef;

	struct Slot
	{
		ID id;
		ID data_id;
	};

	template<typename T>
	struct ObjectSlot
	{
		ObjectSlot(ID id_, T* object_)
			: id(id_), object(object_)
		{ }
		
		ID id;
		T* object;
	};

	struct GenericProvider
	{
		virtual ~GenericProvider()
		{
		
		}

		virtual void* get(civ::ID id) = 0;

		[[nodiscard]]
		virtual bool isValid(civ::ID, uint64_t validity_id) const = 0;
	};

	template<typename T>
	struct ObjectSlotConst
	{
		ObjectSlotConst(ID id_, const T* object_)
			: id(id_), object(object_)
		{ }

		ID id;
		const T* object;
	};

	struct SlotMetadata
	{
		ID rid;
		ID op_id;
	};

	template<typename T>
	struct Vector : public GenericProvider
	{
		Vector() : data_size(0), op_count(0)
		{}

		~Vector() override
		{
			// Since we already explicitly destroyed objects >= data_size index
			// the compiler will complain when double freeing these objects.
			// The quick fix for now is to fill these places with default initialized objects
			const uint64_t capacity = data.size();
			for (uint64_t i{data_size}; i < capacity; ++i)
			{
				new(&data[i]) T();
			}
 		}

		template<typename... Args>
		ID emplace_back(Args&&... args);
		ID push_back(const T& obj);
		[[nodiscard]]
		ID getNextId() const;
		void erase(ID id);
		template<typename TPredicate>
		void remove_if(TPredicate&& f);
		void clear();

		T& operator[](ID id);
		const T& operator[](ID id) const;

		Ref<T> getRef(ID id);
		template<typename U>
		PRef<U> getPRef(ID id);

		T& getDataAt(uint64_t i);

		[[nodiscard]]
		bool isValid(ID id, ID validity) const override;
		[[nodiscard]]
		uint64_t getOperationId(ID id) const;

		ObjectSlot<T> getSlotAt(uint64_t i);
		ObjectSlotConst<T> getSlotAt(uint64_t i) const;

		typename std::vector<T>::iterator begin();
		typename std::vector<T>::iterator end();
		typename std::vector<T>::const_iterator begin() const;
		typename std::vector<T>::const_iterator end() const;

		[[nodiscard]]
		uint64_t size() const;

		[[nodiscard]]
		ID getValidityID(ID id) const;

	public:
		std::vector<T> data;
		std::vector<uint64_t> ids;
		std::vector<SlotMetadata> metadata;
		uint64_t data_size;
		uint64_t op_count;

		[[nodiscard]]
		bool isFull() const;

		[[nodiscard]]
		ID getID(uint64_t i) const;
		
		[[nodiscard]]
		uint64_t getDataID(ID id) const;
		Slot createNewSlot();
		Slot getFreeSlot();
		Slot getSlot();
		SlotMetadata& getMetadataAt(ID id);
		const T& getAt(ID id) const;
		[[nodiscard]]
		void* get(civ::ID id) override;

		template<typename TCallback>
		void foreach(TCallback&& callback);

		template<class U> friend struct PRef;
	};

	template<typename T>
	template<typename ...Args>
	inline uint64_t Vector<T>::emplace_back(Args&& ...args)
	{
		const Slot slot = getSlot();
		new(&data[slot.data_id]) T(std::forward<Args>(args)...);
		return slot.id;
	}

	template<typename T>
	inline uint64_t Vector<T>::push_back(const T& obj)
	{
		const Slot slot = getSlot();
		data[slot.data_id] = obj;
		return slot.id;
	}

	template<typename T>
	inline void Vector<T>::erase(ID id)
	{
		// retrieve the object position in data
		const uint64_t data_index = ids[id];
		// check if the object has been already erased
		if (data_index >= data_size) return;
		// destroy the object
		data[data_index].~T();
		// swap the object at the end
		--data_size;
		const uint64_t last_id = metadata[data_size].rid;
		std::swap(data[data_size], data[data_index]);
		std::swap(metadata[data_size], metadata[data_index]);
		std::swap(ids[last_id], ids[id]);
		// invalidate the operation
		metadata[data_size].op_id = ++op_count;
	}

	template<typename T>
	inline T& Vector<T>::operator[](ID id)
	{
		return const_cast<T&>(getAt(id));
	}

	template<typename T>
	inline const T& Vector<T>::operator[](ID id) const
	{
		return getAt(id);
	}

	template<typename T>
	inline ObjectSlot<T> Vector<T>::getSlotAt(uint64_t i)
	{
		return ObjectSlot<T>(metadata[i].rid, &data[i]);
	}

	template<typename T>
	inline ObjectSlotConst<T> Vector<T>::getSlotAt(uint64_t i) const
	{
		return ObjectSlotConst<T>(metadata[i].rid, &data[i]);
	}

	template<typename T>
	inline Ref<T> Vector<T>::getRef(ID id)
	{
		return Ref<T>(id, this, metadata[ids[id]].op_id);
	}

	template<typename T>
	template<typename U>
	inline PRef<U> Vector<T>::getPRef(ID id)
	{
		return PRef<U>(id, this, metadata[ids[id]].op_id);
	}

	template<typename T>
	inline T& Vector<T>::getDataAt(uint64_t i)
	{
		return data[i];
	}

	template<typename T>
	inline uint64_t Vector<T>::getID(uint64_t i) const
	{
		return metadata[i].rid;
	}

	template<typename T>
	inline uint64_t Vector<T>::size() const
	{
		return data_size;
	}

	template<typename T>
	inline typename std::vector<T>::iterator Vector<T>::begin()
	{
		return data.begin();
	}

	template<typename T>
	inline typename std::vector<T>::iterator Vector<T>::end()
	{
		return data.begin() + data_size;
	}

	template<typename T>
	inline typename std::vector<T>::const_iterator Vector<T>::begin() const
	{
		return data.begin();
	}

	template<typename T>
	inline typename std::vector<T>::const_iterator Vector<T>::end() const
	{
		return data.begin() + data_size;
	}

	template<typename T>
	inline bool Vector<T>::isFull() const
	{
		return data_size == data.size();
	}

	template<typename T>
	inline Slot Vector<T>::createNewSlot()
	{
		data.emplace_back();
		ids.push_back(data_size);
		metadata.push_back({ data_size, op_count++ });
		return { data_size, data_size };
	}

	template<typename T>
	inline Slot Vector<T>::getFreeSlot()
	{
		const uint64_t reuse_id = metadata[data_size].rid;
		metadata[data_size].op_id = op_count++;
		return { reuse_id, data_size };
	}

	template<typename T>
	inline Slot Vector<T>::getSlot()
	{
		const Slot slot = isFull() ? createNewSlot() : getFreeSlot();
		++data_size;
		return slot;
	}

	template<typename T>
	inline SlotMetadata& Vector<T>::getMetadataAt(ID id)
	{
		return metadata[getDataID(id)];
	}

	template<typename T>
	inline uint64_t Vector<T>::getDataID(ID id) const
	{
		return ids[id];
	}

	template<typename T>
	inline const T& Vector<T>::getAt(ID id) const
	{
		return data[getDataID(id)];
	}

	template<typename T>
	inline bool Vector<T>::isValid(ID id, ID validity) const
	{
		return validity == metadata[getDataID(id)].op_id;
	}

	template<typename T>
	inline uint64_t Vector<T>::getOperationId(ID id) const
	{
		return metadata[getDataID(id)].op_id;
	}

	template<typename T>
	template<typename TPredicate>
	void Vector<T>::remove_if(TPredicate&& f)
	{
		for (uint64_t data_index{ 0 }; data_index < data_size;)
		{
			if (f(data[data_index]))
			{
				erase(metadata[data_index].rid);
			}
			else
			{
				data_index++;
			}
		}
	}

	template<typename T>
	ID Vector<T>::getNextId() const
	{
		return isFull() ? data_size : metadata[data_size].rid;
	}

	template<typename T>
	void* Vector<T>::get(civ::ID id)
	{
		return static_cast<void*>(&data[ids[id]]);
	}

	template<typename T>
	void Vector<T>::clear()
	{
		ids.clear();
		data.clear();
		metadata.clear();
		for (SlotMetadata& slm : metadata)
		{
			slm.rid = 0;
			slm.op_id = ++op_count;
		}
		data_size = 0;
	}

	template<typename T>
	template<typename TCallback>
	void Vector<T>::foreach(TCallback&& callback)
	{
		// use index based for to all data creation during iteration
		const uint64_t current_size = data_size;
		for (uint64_t i{0}; i < current_size; i++)
		{
			callback(data[i]);
		}
	}

	template<typename T>
	ID Vector<T>::getValidityID(ID id) const
	{
		return metadata[ids[id]].op_id;
	}

	template<typename T>
	struct Ref
	{
		Ref() : id(0), array(nullptr), validity_id(0)
		{ }

		Ref(ID id_, Vector<T>* a, ID vid) : id(id_), array(a), validity_id(vid)
		{ }

		T* operator->()
		{
			return &(*array)[id];
		}

		const T* operator->() const
		{
			return &(*array)[id];
		}

		T& operator*()
		{
			return (*array)[id];
		}

		const T& operator*() const
		{
			return (*array)[id];
		}

		civ::ID getID() const
		{
			return id;
		}

		explicit
			operator bool() const
		{
			return array && array->isValid(id, validity_id);
		}

	public:
		ID id;
		Vector<T>* array;
		ID validity_id;
	};

	template<typename T>
	struct PRef
	{
		using ProviderCallback = T*(*)(ID, GenericProvider*);

		PRef() : id_(0), provider_callback_(nullptr), provider_(nullptr), validity_id_(0)
		{ }

		template<typename U>
		PRef(ID index, Vector<U>* a, ID vid)
			: id_(index), 
			provider_callback_{PRef<T>::get<U>},
			provider_(a),
			validity_id_(vid)
		{ }

		template<typename U>
		PRef(const PRef<U>& other)
			: id_(other.id_),
			provider_callback_{ PRef<T>::get<U> },
			provider_(other.provider_),
			validity_id_(other.validity_id_)
		{ }

		template<typename U>
		static T* get(ID index, GenericProvider* provider)
		{
			return dynamic_cast<T*>(static_cast<U*>(provider->get(index)));
		}

		T* operator->()
		{
			return provider_callback_(id_, provider_);
		}

		T& operator*() 
		{
			return *provider_callback_(id_, provider_);
		}

		const T& operator*() const
		{
			return *provider_callback_(id_, provider_);
		}

		civ::ID getID() const
		{
			return id_;
		}

		explicit
			operator bool() const
		{
			return provider_ && provider_->isValid(id_, validity_id_);
		}
	private:
		ID id_;
		ProviderCallback provider_callback_;
		GenericProvider* provider_;
		uint64_t validity_id_;

		template<class U> friend struct PRef;
		template<class U> friend struct Vector;
	};
}


#endif // !INDEXVECTOR_H
