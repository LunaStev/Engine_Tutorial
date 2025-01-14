﻿using System;
using System.Runtime.CompilerServices;

namespace Z
{
    internal class Internal
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void InternalCallPRT(string a);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void InternalCallWARN(ref Vector3 a);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void InternalCallDot(ref Vector3 a, ref Vector3 b, out Vector3 c);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetTranslation(ulong id, out Vector3 vec);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTranslation(ulong id, ref Vector3 vec);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsKeyPressed(KeyCode key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Entity_HasComponent(ulong id, Type component);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_GetVelocity(ulong id, out Vector2 vec);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_SetVelocity(ulong id, ref Vector2 vec);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_ApplyForce(ulong id, ref Vector2 force, ref Vector2 point, bool wake);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_ApplyForceCenter(ulong id, ref Vector2 force, bool wake);
    }
}