= 最初の例: Option

//lead{
"I call it my billion-dollar mistake. It was the invention of the null reference in 1965." Charles Antony Richard Hoare
//}

== null の問題点を探る

いくつかのプログラミング言語では、値が存在しないことを ``null`` キーワードで表現します。
そういった言語で意図せず ``null`` を束縛した変数にアクセスした場合、例外やエラーが発生します。
例外発生を回避するためには、事前に ``null`` チェックを行い、束縛されている値が ``null`` であることを確認する必要があります。
そして、この方法にはにはいくつかの問題点があります。

 1. 利用者側はその関数を適用することによって ``null`` が束縛、代入される可能性を（すぐには）判別できない
 2. ``null`` だった場合の処理を忘れやすい
 3. ``null`` チェックによりネストが深くなりがち

ネストはともかく、他の二項目に関しては切実な問題なので、これらの問題の解決策としてよく知られている方法を見ていきましょう。
null 対する解決方法はいくつか存在します。

 1. Null Objectパターン
 2. Safe Navigation Operator（Groovy）、Null-conditional Operator（C# 6.0）
 3. 型による表現

Null Objectパターンは、``null``の代わりに 値を持たない、処理が空のオブジェクトを使用するパターンです。
ただし、Null Object の実装次第では挙動が変わってしまうこともあるため、完全な代替とは言い切れません。
また、機械的な強制力はないため、初期化時の代入を忘れる可能性があります。

Safe Navigation OperatorやNull-conditional Operatorは、左辺が``null``だった場合は``null``を返し、それ以外の場合は メソッドを呼び出す演算子です。
既存のコードをほとんど書き換えることなく利用できるので、便利なことは確かです。
しかし、Null Objectと同様に強制力はないため、うっかりミスは防げません。
また、この演算子の戻り値``null``がメソッドの戻り値として返される可能性は、0ではありません。

型による表現では、判別共用体（もしくはそれと同等の機能）によって”値をもたない状態”と”何かデータをもつ状態”を表します。
これにより、 値が存在しない可能性を型によって伝搬させることができ、この型を返す関数を利用するユーザーに対して何かしらの処理を行うことを強制できます。
なお、.NET FrameworkのNullable型も``null``を表現する型ですが、Nullableは構造体のみを対象としています。
対して、判別共用体による表現では任意の型を対象にできます。

== 値が存在しないかもしれない型

F#では、値が存在するかもしれない値を@<code>{Option}型で表現します。
Option型は標準で提供されており、およそ次のような定義です@<fn>{definition}。

//footnote[definition][公式実装では.NET Frameworkとの相互運用の関係でいくつかのAttributeも定義されているのですが、本資料では不要な情報なので割愛します。]

//emlist{
type Option<'T> = Some of 'T | None
//}

@<code>{Option}型は、任意の値 をもつ@<code>{Some}と、値を持たない@<code>{None}という2つのケース識別子から成ります。

ここで、@<code>{db}から指定したキーに対応する値を取得する操作について考えてみましょう。

//emlist{
let db = [ ("x", 1); ("y", 2); ("z", 3) ]
//}

指定したキーが@<code>{db}に登録されていないときは、返す値がありません。
そこで、@<code>{Option}型の出番です。

//emlist{
let rec tryFind key db =
  match db with
  | [] -> None
  | (k, v) :: xs when k = key -> Some v
  | _ :: xs -> tryFind key xs
//}

指定したキーに一致する値が見つからない際に@<code>{None}を返すことで、値が存在しないことを伝えられるようになりました。

//emlist{
tryFind "y" db // equal (Some 2)
tryFind "w" db // equal None
//}

@<code>{db}からふたつの値を取得してその合計を返す関数を定義してみましょう。

//emlist{
let sum a b db =
  match tryFind a db with
  | Some x ->
    match tryFind b db with
    | Some y -> Some (x + y)
    | None -> Nothing
  | Nothing -> Nothing

sum "x" "z" db // equal (Some 4)
sum "x" "w" db // equal None
sum "v" "y" db // equal None
//}

パターンマッチを使うことで、安全に値を取得して計算できるようになりました。

しかし、不満な点もあります。
パターンマッチがネストしてしまう点と、@<code>{None}にマッチした際@<code>{None}を返す、という定型的なコードが増えてしまう点です。

== 理想1: 定型的なコードを減らす

ここで共通する操作は、@<code>{Some}なら何かしらの計算を適用して @<code>{Option}型を返し、@<code>{None} ならば @<code>{None} を返す部分です。
幸いなことに、このような定型コードはF#標準の@<code>{Option.bind}関数を使うことで無くすことができます。

//emlist{
let sum a b db =
  tryFind a db
  |> Option.bind (fun x ->
    tryFind b db
    |> Option.bind (fun y ->
      Some (x + y)
    )
  )
//}

== 理想2: 手続き的な書式で記述する

本章の序盤で”ネストはともかく”などと書いてしまいましたが、ネストを少なくしたほうが可読性の向上に繋がるのは確かです。
欲をいえば、@<code>{let}と同等のキーワードを利用して束縛を表現できればもっとわかりやすくなることでしょう。
F#では、コンピュテーション式でこれを実現します。

//emlist{
// コンピュテーション式用のビルダークラス
type OptionBuilder () =
  member this.Return(x) = Some x
  member this.ReturnFrom(x: Option<_>) = x
  member this.Bind(x, f) = Option.bind f x

// ビルダーインスタンス
let option = OptionBuilder()
//}

@<code>{Option.bind}に対応する@<code>{Bind}メソッドによって@<code>{let!}、値を@<code>{Some}で包んで返す@<code>{Return} メソッドによって@<code>{return}、@<code>{Option}型の値をそのまま返す@<code>{ReturnFrom}メソッドによって@<code>{return!}キーワードがコンピュテーション式内で利用可能になります。

準備が整ったので、再び@<code>{sum}関数を書き換えてみましょう。

//emlist{
let sum a b db =
  option {
    let! x = tryFind a db
    let! y = tryFind b db
    return x + y
  }
//}

コンピュテーション式の導入によって、ネストを平坦化することができました。

== モナドとコンピュテーション式

@<code>{Bind}や@<code>{Return}といったメソッド名を見てピンときた方もいらっしゃることでしょう。
コンピュテーション式は、モナド則を満たす型の計算を手続き的な記述へと置き換える際にとても都合のよい仕組みなのです。

作成したコンピュテーション式がモナド則を満たすことを確認してみましょう。
コンピュテーション式を使わなかった場合のコードは次のとおりです。

//emlist{
let point a = Some a

let rightIdentityLaw a =
  Option.bind point a = a

let leftIdentityLaw f a =
  Option.bind f (point a) = f a

let associativeLaw f g fa =
  Option.bind g (Option.bind f fa)
    = Option.bind (fun a -> Option.bind g (f a)) fa
//}

@<code>{point}は任意の値を@<code>{Option}型に持ち上げるための補助関数です。

左辺をコンピュテーション式を用いた実装に変更してみましょう。

//emlist{
let rightIdentityLaw a =
  let left = option {
    let! fa = a
    return! point fa
  }
  left = a

let leftIdentityLaw f a =
  let left = option {
    let! fa = point a
    return! f fa
  }
  left = f a

let associativeLaw f g fa =
  let left = option {
    let! a = fa
    let! b = f a
    return! g b
  }
  left = Option.bind (fun a -> Option.bind g (f a)) fa
//}

