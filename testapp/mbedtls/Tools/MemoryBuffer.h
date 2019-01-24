#pragma once

#define ENABLE_ENCRYPTION 1

class CMemoryBuffer
{
private:
	std::vector<BYTE> m_buffer;
	size_t m_currentPos;
public:
	CMemoryBuffer( DWORD size = 0x20 ) {
		m_buffer.reserve(size);
		m_currentPos = 0;
	}
	CMemoryBuffer& CMemoryBuffer::operator=(CMemoryBuffer rhs) {
		m_buffer.swap(rhs.m_buffer);
		return *this;
	}

	const BYTE& operator[](size_t idx) const					{ return m_buffer[idx];			}
	BYTE& operator[](size_t idx)								{ return m_buffer[idx];			}

	std::vector<BYTE>::iterator begin( void )					{ return m_buffer.begin();		}
	std::vector<BYTE>::iterator end( void )						{ return m_buffer.end();		}
	std::vector<BYTE>::const_iterator begin( void ) const		{ return m_buffer.begin();		}
	std::vector<BYTE>::const_iterator end( void ) const			{ return m_buffer.end();		}

	size_t length( void ) const									{ return m_buffer.size();		}
	size_t capacity( void ) const								{ return m_buffer.capacity();	}
	void resize( size_t len, BYTE val )							{ m_buffer.resize( len, val );	}
	void reserve( size_t len )									{ m_buffer.reserve( len );		}
	void clear( void )											{ m_buffer.clear();				}
	void shrink_to_fit( void )									{ m_buffer.shrink_to_fit();		}

	size_t read( const PBYTE data, size_t len ) {

	}

	size_t position( void ) { return m_currentPos; }
	void rewind( size_t pos = 0 ) {
		if( pos >= length() ) pos = length() - 1;
		m_currentPos = pos;
	}

	void insert( size_t offset, const CMemoryBuffer& data ) {
		size_t len = data.length();
		m_buffer.insert( m_buffer.begin() + offset, data.begin(), data.end() );
	}
	void insert( size_t offset, const PBYTE data, size_t len ) {
		m_buffer.insert( m_buffer.begin() + offset, data, data + len );
	}
	void insert( size_t offset, const std::string& str ) {
		insert( offset, (BYTE*)str.c_str(), str.length() );
	}
	
	void prepend( const CMemoryBuffer& data ) {
		m_buffer.insert( m_buffer.begin(), data.begin(), data.end() );
	}
	void prepend( const BYTE * data, size_t len ) {
		m_buffer.insert( m_buffer.begin(), data, data + len );
	}
	void prepend( const std::string& str ) {
		prepend( (BYTE*)str.c_str(), str.length() );
	}
	
	void append( const CMemoryBuffer& data ) {
		m_buffer.insert( m_buffer.end(), data.begin(), data.end() );
	}
	void append( const BYTE * data, size_t len ) {
		m_buffer.insert( m_buffer.end(), data, data + len );
	}
	void append( const std::string& str ) {
		append( (BYTE*)str.c_str(), str.length() );
	}

	void fill( BYTE val ) {
		std::fill( m_buffer.begin(), m_buffer.end(), val );
	}
	void fill( BYTE val, size_t len ) {
		if( len > m_buffer.size() ) m_buffer.resize( len );
		std::fill( m_buffer.begin(), m_buffer.end(), val );
	}
	void random( size_t len ) {
		if( len > m_buffer.size() ) m_buffer.resize( len );
		XNetRandom( &m_buffer[0], len );
	}
	void random( size_t offset, size_t len ) {
		if( offset + len > m_buffer.size() ) m_buffer.resize( offset + len );
		XNetRandom( &m_buffer[offset], len );
	}
	void copy( size_t offset, const CMemoryBuffer& data ) {
		if( data.length() > m_buffer.size() ) m_buffer.resize( data.length() );
		std::copy( data.begin(), data.end(), &m_buffer[offset] );
	}
	void copy( size_t offset, const PBYTE data, size_t len ) {
		if( len > m_buffer.size() ) m_buffer.resize( len );
		std::copy( data, data + len, &m_buffer[offset] );
	}
	CMemoryBuffer slice( size_t offset, size_t len ) const {
		CMemoryBuffer part(len);
		part.copy( 0, (BYTE*)&m_buffer[offset], len );
		return part;
	}
	CMemoryBuffer slice( size_t offset ) const
	{
		CMemoryBuffer part( m_buffer.size() - offset );
		part.copy( 0, (BYTE*)&m_buffer[offset], m_buffer.size() - offset );
	}
	void print_buffer() const {
		const char * buffer = (const char*)&m_buffer[0];
		size_t len = m_buffer.size();
		char outstr[1025];
		while( len > 0 )
		{
			int bytes = len < 1024 ? len : 1024;
			outstr[bytes] = 0x0;
			memcpy(outstr, buffer, bytes );
			printf("%s", outstr );

			len -= bytes;
			buffer += bytes;
		}
		printf("\n**** EOF ****\n" );
	}
};